#include "log/log.h"

int main(int argc, char *argv[])
{
    log_set_level(LOG_TRACE);
    
    log_trace("log_trace");
    log_debug("log_debug");
    log_info("log_info");
    log_warn("log_warn");
    log_error("log_error");
    log_fatal("log_fatal");

    return 0;
}
