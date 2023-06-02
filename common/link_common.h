////////////////////////////////////////////////////////////////////////////////
//
// Common Link Stuff
//
////////////////////////////////////////////////////////////////////////////////

// Global buffer size
#define GLOBAL_SIZE (16*1024)

// Bus Pico ROM size
#define ROM_SIZE  GLOBAL_SIZE

// SD Pico RAM size
#define RAM_SIZE  GLOBAL_SIZE

// Link buffer size
#define MD_ROM_SIZE GLOBAL_SIZE

////////////////////////////////////////////////////////////////////////////////

#define LINK_BYTE_NONE  600
#define LINK_BYTE_ACK   601


typedef int boolean;
typedef uint8_t BYTE;
typedef void (*COMPLETION_FN)(void);

typedef void (*MDFPTR)();

extern uint8_t memory_data_ram_data[MD_ROM_SIZE];
extern int memory_data_cmd_count;
extern MDFPTR memory_data_handler;
extern uint64_t md_start_time;
extern uint64_t md_elapsed_time;

extern int linkout_sm;
extern PIO linkout_pio;
extern int linkin_sm;
extern PIO linkin_pio;

#define LINK_CMD_IDLE          0xaa   // Do nothing
#define LINK_CMD_MEMBLK        0xab   // Memory block to write to copy
#define LINK_CMD_ECHO          0xac   // Echo the payload back
#define LINK_CMD_ECHO_REPLY    0xad   // Reply from an echo command
#define LINK_CMD_TEST_INT      0xae   // Send a test integer
#define LINK_ADD_MEMORY_DATA   4      // Extra bytes in payload

// Link FSM stimulii
#define LFSM_ACK  1
#define LFSM_DATA 2
#define LFSM_SEND_IDLE 3

// Link FSM states
#define LFSM_STATE_IDLE      1
#define LFSM_STATE_WAIT_ACK  2

extern int link_data_test_int;

int get_data_from_link(void);
void send_ack_to_link(void);
void send_byte_to_link(int data);
uint32_t picoputerlinkin_get(PIO pio, uint sm);
void link_process(void);
void lfsm_idle_fn(void);
void lfsm_ack_received(void);
void send_byte_string(BYTE *string, COMPLETION_FN done);
void link_fsm_stimulus(int what, int stim);



