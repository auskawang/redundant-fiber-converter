#ifndef CLI_H
#define CLI_H
#include <stdint.h>
void cli_init(void);
void cli_process(void);
/* Main-loop byte input, also usable by transport-independent tests. */
void cli_feed_byte(uint8_t byte);
#endif
