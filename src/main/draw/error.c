#include "pe1/boot_services.h"
#include "pe1/psyq_bios.h"

void Draw_ReportError(char *message, char *detail, int value, unsigned char severity) {
    int index = severity & 0xFF;

    printf(g_ErrorMessageFormat, g_ErrorSeverityNames[index], message);
    printf(g_ErrorDetailFormat, detail, value);

    if (index == 1) {
        while (1) {
        }
    }
}
