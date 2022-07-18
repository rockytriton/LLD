#include <mem.h>
#include <peripherals/base.h>
#include <mm.h>
#include <mmu.h>
#include <printf.h>

static u16 mem_map [ PAGING_PAGES ] = {0,};

void *allocate_memory(int bytes) {
    int pages = bytes / PAGE_SIZE;

    if (bytes % PAGE_SIZE) {
        pages++;
    }

    return get_free_pages(pages);
}

void free_memory(void *base) {
    u64 page_num = (((u64)base) - LOW_MEMORY) / PAGE_SIZE;
    int pages = mem_map[page_num];

    printf("free_memory at address %X page num: %d pages: %d\n", base, page_num, pages);

    for (int i=0; i<pages; i++) {
        mem_map[page_num + i] = 0;
    }
}

void *get_free_pages(int num_pages) {
    int start_index = 0;
    int count = 0;

    for (int i=0; i<PAGING_PAGES; i++) {
        if (mem_map[i] == 0) {
            //not yet allocated...
            if (!count) {
                start_index = i;
            }

            count++;

            if (count == num_pages) {
                mem_map[start_index] = count; //number of pages allocated

                for (int c=1; c<count; c++) {
                    mem_map[c + start_index] = 1;
                }

                void *p = (void *)(LOW_MEMORY + (start_index * PAGE_SIZE));

                printf("get_free_pages returning %d pages starting at %d at address %X\n", count, start_index, p);

                return p;
            }
        } else {
            count = 0;
        }
    }
}

void *memcpy(void *dest, const void *src, u32 n) {
    //simple implementation...
    u8 *bdest = (u8 *)dest;
    u8 *bsrc = (u8 *)src;

    for (int i=0; i<n; i++) {
        bdest[i] = bsrc[i];
    }

    return dest;
}

#define TD_VALID                   (1 << 0)
#define TD_BLOCK                   (0 << 1)
#define TD_TABLE                   (1 << 1)
#define TD_ACCESS                  (1 << 10)
#define TD_KERNEL_PERMS            (1L << 54)
#define TD_INNER_SHARABLE          (3 << 8)

#define TD_KERNEL_TABLE_FLAGS      (TD_TABLE | TD_VALID)
#define TD_KERNEL_BLOCK_FLAGS      (TD_ACCESS | TD_INNER_SHARABLE | TD_KERNEL_PERMS | (MATTR_NORMAL_NC_INDEX << 2) | TD_BLOCK | TD_VALID)
#define TD_DEVICE_BLOCK_FLAGS      (TD_ACCESS | TD_INNER_SHARABLE | TD_KERNEL_PERMS | (MATTR_DEVICE_nGnRnE_INDEX << 2) | TD_BLOCK | TD_VALID)

#define MATTR_DEVICE_nGnRnE        0x0
#define MATTR_NORMAL_NC            0x44
#define MATTR_DEVICE_nGnRnE_INDEX  0
#define MATTR_NORMAL_NC_INDEX      1
#define MAIR_EL1_VAL               ((MATTR_NORMAL_NC << (8 * MATTR_NORMAL_NC_INDEX)) | MATTR_DEVICE_nGnRnE << (8 * MATTR_DEVICE_nGnRnE_INDEX))

#define ID_MAP_PAGES           6
#define ID_MAP_TABLE_SIZE      (ID_MAP_PAGES * PAGE_SIZE)
#define ENTRIES_PER_TABLE      512
#define PGD_SHIFT              (PAGE_SHIFT + 3 * TABLE_SHIFT)
#define PUD_SHIFT              (PAGE_SHIFT + 2 * TABLE_SHIFT)
#define PMD_SHIFT              (PAGE_SHIFT + TABLE_SHIFT)
#define PUD_ENTRY_MAP_SIZE     (1 << PUD_SHIFT)

#define BLOCK_SIZE 0x40000000

void create_table_entry(u64 tbl, u64 next_tbl, u64 va, u64 shift, u64 flags) {
    u64 table_index = va >> shift;
    table_index &= (ENTRIES_PER_TABLE - 1);
    u64 descriptor = next_tbl | flags;
    *((u64 *)(tbl + (table_index << 3))) = descriptor;
}

void create_block_map(u64 pmd, u64 vstart, u64 vend, u64 pa) {
    vstart >>= SECTION_SHIFT;
    vstart &= (ENTRIES_PER_TABLE -1);

    vend >>= SECTION_SHIFT;
    vend--;
    vend &= (ENTRIES_PER_TABLE - 1);

    pa >>= SECTION_SHIFT;
    pa <<= SECTION_SHIFT;

    do {
        u64 _pa = pa;

        if (pa >= DEVICE_START) {
            _pa |= TD_DEVICE_BLOCK_FLAGS;
        } else {
            _pa |= TD_KERNEL_BLOCK_FLAGS;
        }

        *((u64 *)(pmd + (vstart << 3))) = _pa;
        pa += SECTION_SIZE;
        vstart++;
    } while(vstart <= vend);
}

u64 id_pgd_addr();

void init_mmu() {
    u64 id_pgd = id_pgd_addr();

    memzero(id_pgd, ID_MAP_TABLE_SIZE);

    u64 map_base = 0;
    u64 tbl = id_pgd;
    u64 next_tbl = tbl + PAGE_SIZE;

    create_table_entry(tbl, next_tbl, map_base, PGD_SHIFT, TD_KERNEL_TABLE_FLAGS);

    tbl += PAGE_SIZE;
    next_tbl += PAGE_SIZE;

    u64 block_tbl = tbl;

    for (u64 i=0; i<4; i++) {
        create_table_entry(tbl, next_tbl, map_base, PUD_SHIFT, TD_KERNEL_TABLE_FLAGS);

        next_tbl += PAGE_SIZE;
        map_base += PUD_ENTRY_MAP_SIZE;

        block_tbl += PAGE_SIZE;

        u64 offset = BLOCK_SIZE * i;
        create_block_map(block_tbl, offset, offset + BLOCK_SIZE, offset);
    }
}
