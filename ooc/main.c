#include "log.h"
#include "flog.h"

#include <stdio.h>

//macros for less typing...
#define ILOG(log, msg) (log)->interface->info(log, msg)
#define ELOG(log, msg) (log)->interface->error(log, msg)

int main() {
    //
    //console log sample
    //
    log_interface *clog = conlog.interface;

    //not really necessary for console log, but...
    clog->init(&conlog, 0);

    //long winded usage
    conlog.interface->info(&conlog, "Info message 1");
    conlog.interface->info(&conlog, "Error message 1");

    //shorter usage
    clog->info((void *)clog, "Info message 2");
    clog->error((void *)clog, "Error message 2");

    //macro usage
    ILOG(&conlog, "Info message 3");
    ELOG(&conlog, "Error message 3");

    //
    //file log samples
    //

    //first setup 2 separate log files.
    logger f1;
    logger f2;

    //create the loggers...
    filelog_create(&f1, "file1.log");
    filelog_create(&f2, "file2.log");

    log_interface *flog1 = f1.interface;
    log_interface *flog2 = f2.interface;

    //shorter usage
    flog1->info(&f1, "Info message 1");
    flog2->info(&f2, "Info message 1");
    flog1->error(&f1, "Error message 1");
    flog2->error(&f2, "Error message 1");

    //macro usage
    ILOG(&f1, "Info message 2");
    ILOG(&f2, "Info message 2");

    ELOG(&f1, "Error message 2");
    ELOG(&f2, "Error message 2");

    flog1->close(&f1);
    flog2->close(&f2);

    return 0;
}
