#include "cli.h"
#include "bxp_uart.h"
#include "bxp_gpio.h"
#include "bxp_i2c.h"
#include "bxp_time.h"
#include "diagnostics.h"
#include "optical_health.h"
#include "path_manager.h"
#include "peer_comm.h"
#include <stdio.h>
#include <string.h>
static char line[96];
static unsigned used;
static int overflow, cr, announced;
static void out(const char *s) { (void)bxp_uart_write((const uint8_t *)s,(uint16_t)strlen(s)); }
static const char *path_name(path_state_t s) {
 switch (s) {
 case PATH_STATE_INIT: return "NOT INITIALIZED";
 case PATH_STATE_PRIMARY_ACTIVE: return "PRIMARY_ACTIVE";
 case PATH_STATE_BACKUP_STANDBY: return "BACKUP_STANDBY";
 case PATH_STATE_FAILOVER_IN_PROGRESS: return "FAILOVER_IN_PROGRESS";
 case PATH_STATE_SECONDARY_ACTIVE: return "SECONDARY_ACTIVE";
 case PATH_STATE_FAULT: return "FAULT";
 default: return "UNKNOWN";
 }
}
static void execute(void) {
 char text[160];
 if (!strcmp(line,"help")) out("help | system status | sfp status | switch status | failover status | peer status | diagnostics\r\n");
 else if (!strcmp(line,"system status")) {
  snprintf(text,sizeof text,"Uptime: %lu ms; GPIO: %s; I2C: %s; UART: %s\r\n",(unsigned long)bxp_time_ms(),bxp_gpio_is_ready()?"configured (unverified)":"UNAVAILABLE",bxp_i2c_is_ready()?"bound":"UNAVAILABLE",bxp_uart_is_ready()?"bound":"UNAVAILABLE"); out(text);
 } else if (!strcmp(line,"sfp status")) {
  optical_status_t st=optical_health_get_status();
  if (st==OPTICAL_STATUS_UNAVAILABLE) out("SFP: UNAVAILABLE (mapping/telemetry pending)\r\n");
  else if (st==OPTICAL_STATUS_DISCONNECTED) out("SFP: DISCONNECTED\r\n");
  else { sfp_ddm_data_t d; optical_health_get_latest(&d); snprintf(text,sizeof text,"SFP cached status=%u, temperature=%ld mC, voltage=%lu mV; optical power units NOT VALIDATED\r\n",(unsigned)st,(long)d.temperature_millicelsius,(unsigned long)d.supply_voltage_mv); out(text); }
 } else if (!strcmp(line,"switch status")) out("Switch: NOT INITIALIZED; register protocol/map pending validation\r\n");
 else if (!strcmp(line,"failover status")) { snprintf(text,sizeof text,"Failover software state=%s; hardware path UNAVAILABLE (switch not initialized)\r\n",path_name(path_manager_get_state())); out(text); }
 else if (!strcmp(line,"peer status")) out(peer_comm_is_peer_alive()?"Peer: heartbeat received; Ethernet transport still incomplete\r\n":"Peer: UNAVAILABLE / no current heartbeat; Ethernet transport incomplete\r\n");
 else if (!strcmp(line,"diagnostics")) {
  out("Hardware counters: UNAVAILABLE (collection not implemented)\r\n");
  for (uint16_t i=0;i<diagnostics_log_count();++i) { diag_log_entry_t e; if (!diagnostics_get_log(i,&e)) { snprintf(text,sizeof text,"[%lu] severity=%u %s\r\n",(unsigned long)e.timestamp_ms,(unsigned)e.severity,e.message?e.message:""); out(text); } }
 } else if (used) out("Unknown command. Type help.\r\n");
}
void cli_init(void) { used=0; overflow=cr=announced=0; }
void cli_feed_byte(uint8_t c) {
 if (c=='\n' && cr) { cr=0; return; }
 cr=(c=='\r');
 if (c=='\r' || c=='\n') {
  out("\r\n");
  if (overflow) out("ERROR: line too long; command discarded\r\n");
  else { while (used && line[used-1]==' ') --used; line[used]=0; execute(); }
  used=0; overflow=0; out("> "); return;
 }
 if (overflow) return;
 if (c==8 || c==127) { if (used) { --used; out("\b \b"); } return; }
 if (c=='\t') c=' ';
 if (c<32 || c>126) return;
 if (c==' ' && (!used || line[used-1]==' ')) return;
 if (used==sizeof line-1) { overflow=1; return; }
 line[used++]=(char)c;
 char echo[2]={(char)c,0}; out(echo);
}
void cli_process(void) {
 if (!bxp_uart_is_ready()) return;
 if (!announced) { out("Media Converter CLI (bring-up)\r\n> "); announced=1; }
 /* Bound RX work so other cooperative tasks get a turn. */
 for (unsigned i=0;i<32;++i) { uint8_t c; if (bxp_uart_read(&c)!=0) break; cli_feed_byte(c); }
}
