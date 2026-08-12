#include "Driver.h"

void print_kd(_In_z_ _Printf_format_string_ PCSTR format, ...) {
    va_list args;
    va_start(args, format);

    vDbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, format, args);

    va_end(args);
}