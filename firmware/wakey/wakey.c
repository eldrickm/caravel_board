#include "../defs_mpw-two-mfix.h"
#include "../print_io.h"


// ============================================================================
// WAKEY WAKEY DEFINITIONS
// ============================================================================
// Define Wishbone Addresses in CFG
#define cfg_reg_addr   (*(volatile uint32_t*)0x30000000)
#define cfg_reg_ctrl   (*(volatile uint32_t*)0x30000004)
#define cfg_reg_data_0 (*(volatile uint32_t*)0x30000008)
#define cfg_reg_data_1 (*(volatile uint32_t*)0x3000000C)
#define cfg_reg_data_2 (*(volatile uint32_t*)0x30000010)
#define cfg_reg_data_3 (*(volatile uint32_t*)0x30000014)
// ============================================================================


// ============================================================================
// FIRMWARE ROUTINES
// ============================================================================
void set_gpio(int pin)
{
    (volatile uint32_t) ((reg_mprj_datal) |= pin);
}


void clear_gpio(int pin)
{
    (volatile uint32_t) ((reg_mprj_datal) &= ~(pin));
}
// ============================================================================


// ============================================================================
// CFG METHODS
// ============================================================================
/*
 * cfg_store()
 * ----------------------------------------------------------------------------
 * store 4 bytes to Wakey Wakey memory via wishbone
 * 
 * addr is a 32b address in the Wakey Wakey address space
 * data_3 is MSB
 * data_2
 * data_1
 * data_0 is LSB
 */
void cfg_store(int addr, int data_3, int data_2, int data_1, int data_0)
{
    // write the store address
    cfg_reg_addr = addr;

    // write data words
    cfg_reg_data_0 = data_0;
    cfg_reg_data_1 = data_1;
    cfg_reg_data_2 = data_2;
    cfg_reg_data_3 = data_3;

    // write store command - 0x1
    cfg_reg_ctrl = 0x1;
}


/*
 * cfg_load()
 * ----------------------------------------------------------------------------
 * load 4 bytes from Wakey Wakey memory via wishbone
 * 
 * addr is a 32b address in the Wakey Wakey address space
 * returns a list of values into data
 * where data[3] is the MSB (data_3) and data[0] is the LSB (data_0)
 */
void cfg_load(int addr, int *data)
{
    // write address the load address
    cfg_reg_addr = addr;

    // write the load command - 0x2
    cfg_reg_ctrl = 0x2;

    // INFO: Currently need to wait one clock cycle before read starts
    __asm__("nop\n\t");

    // read the data words
    data[0] = cfg_reg_data_0;
    data[1] = cfg_reg_data_1;
    data[2] = cfg_reg_data_2;
    data[3] = cfg_reg_data_3;
}
// ============================================================================


// ============================================================================
// MEMORY TEST METHODS
// ============================================================================
/*
 * check_output()
 * ----------------------------------------------------------------------------
 * check if 2 int arrays of length 4 contain the same values
 */
bool check_output(int *expected, int *observed, int len) {
    for (int k = 0; k < len; k++) {
        if (expected[k] != observed[k]) {
            print("\n");
            print("EXP: ");
            for (int i = 0; i < len; i++) {
                print_hex(expected[i], 1);
                print(" ");
            }
            print("\n");
            print("OBS: ");
            for (int i = 0; i < len; i++) {
                print_hex(observed[i], 1);
                print(" ");
            }
            print("\n");
            return false;
        }
    }
    return true;
}


/*
 * test_conv1_mem()
 * ----------------------------------------------------------------------------
 * test conv1 addresses via write and readback of sequential values
 */
bool test_conv1_mem()
{
    int readbuf[4]  = {0, 0, 0, 0};

    // sequential writes
    // iterate and write through 4 conv1_mem banks: 3 weights, 1 bias
    for (int bank = 0x00; bank < 0x40; bank += 0x10) {
        // each conv1_mem bank has 8 values
        for (int idx = 0x0; idx < 0x8; idx++) {
            cfg_store(bank + idx, idx + 3, idx + 2, idx + 1, idx);
        }
    }

    // write to conv1_mem shift value
    int idx = 0;
    cfg_store(0x40, idx + 3, idx + 2, idx + 1, idx);

    // sequential read
    for (int bank = 0x00; bank < 0x40; bank += 0x10) {
        for (int idx = 0x0; idx < 0x8; idx++) {
            cfg_load(bank + idx, readbuf);
            int expected[4] = {idx, idx + 1, idx + 2, idx + 3};
            // conv1 data needs to check all 4 bytes
            if (!check_output(expected, readbuf, 4)) {
                return false;
            }
        }
    }

    // read from conv1_mem shift value
    cfg_load(0x40, readbuf);
    int expected[4] = {idx, idx + 1, idx + 2, idx + 3};
    if (!check_output(expected, readbuf, 4)) {
        return false;
    }

    return true;
}


/*
 * test_conv2_mem()
 * ----------------------------------------------------------------------------
 * test conv2 addresses via write and readback of sequential values
 */
bool test_conv2_mem()
{
    int readbuf[4]  = {0, 0, 0, 0};

    // sequential writes
    // iterate and write through 4 conv2_mem banks: 3 weights, 1 bias
    for (int bank = 0x50; bank < 0x90; bank += 0x10) {
        // each conv2_mem bank has 16 values
        for (int idx = 0x00; idx < 0x10; idx++) {
            cfg_store(bank + idx, idx + 3, idx + 2, idx + 1, idx);
        }
    }

    // write to conv2_mem shift value
    int idx = 0;
    cfg_store(0x90, idx + 3, idx + 2, idx + 1, idx);

    // sequential read
    for (int bank = 0x50; bank < 0x90; bank += 0x10) {
        for (int idx = 0x00; idx < 0x10; idx++) {
            cfg_load(bank + idx, readbuf);
            int expected[4] = {idx, idx + 1, idx + 2, idx + 3};
            // conv2 data needs to check only first 2 bytes
            if (!check_output(expected, readbuf, 2)) {
                return false;
            }
        }
    }

    // read from conv2_mem shift value
    cfg_load(0x90, readbuf);
    int expected[4] = {idx, idx + 1, idx + 2, idx + 3};
    if (!check_output(expected, readbuf, 2)) {
        return false;
    }

    return true;
}


/*
 * test_fc_mem()
 * ----------------------------------------------------------------------------
 * test fc_mem addresses via write and readback of sequential values
 */
bool test_fc_mem()
{
    int readbuf[4]  = {0, 0, 0, 0};

    // sequential writes
    // iterate and write through fc_mem weight 0 (0x100) and weight 1 (0x200)
    for (int bank = 0x100; bank < 0x300; bank += 0x100) {
        // each fc_mem weight bank has 208 values
        for (int idx = 0x00; idx < 0xD0; idx++) {
            // fc_mem banks start at 0x50
            cfg_store(bank + idx, idx + 3, idx + 2, idx + 1, idx);
        }
    }

    int idx = 0;
    cfg_store(0x300, idx + 3, idx + 2, idx + 1, idx + 0);  // bias 0
    cfg_store(0x400, idx + 3, idx + 2, idx + 1, idx + 0);  // bias 1

    // sequential read
    for (int bank = 0x100; bank < 0x300; bank += 0x100) {
        for (int idx = 0x00; idx < 0xD0; idx++) {
            cfg_load(bank + idx, readbuf);
            int expected[4] = {idx, idx + 1, idx + 2, idx + 3};
            // fc data needs to check only first 1 byte
            if (!check_output(expected, readbuf, 1)) {
                return false;
            }
        }
    }

    // read from fc_mem bias 0 value
    idx = 0;
    cfg_load(0x300, readbuf);
    int expected[4] = {idx, idx + 1, idx + 2, idx + 3};
    if (!check_output(expected, readbuf, 1)) {
        return false;
    }

    // read from fc_mem bias 1 value
    cfg_load(0x400, readbuf);
    if (!check_output(expected, readbuf, 1)) {
        return false;
    }

    return true;
}
// ============================================================================


// ============================================================================
// LOGIC ANALYZER TEST METHODS
// ============================================================================
/* 
 * la_test() TODO
 * ----------------------------------------------------------------------------
 * test the logic analyzer using the waveforms.
 * Hardcode la_data_out to 'h7777777...
 * in user_project_wrapper.
 */
void la_test() {
    // reg_la2_oenb = reg_la2_iena = 0xFFFFFFFF;    // [95:64]
    reg_la0_data = 0xAAAAAAAA;  // setupt output values
    reg_la1_data = 0xAAAAAAAA;
    reg_la2_data = 0xAAAAAAAA;
    reg_la3_data = 0xAAAAAAAA;
    reg_la0_oenb = reg_la0_iena = 0x0;  // OUTPUT
    reg_la1_oenb = reg_la1_iena = 0x0;
    reg_la2_oenb = reg_la2_iena = 0x0;
    reg_la3_oenb = reg_la3_iena = 0x0;

    reg_la1_oenb = reg_la1_iena = 0xFFFFFFFF;  // INPUT
    int read = reg_la1_data;   // read data (h77777777)
    reg_la3_data = read;   // write on reg 3
    reg_la1_oenb = reg_la1_iena = 0x0;  // test switching speed
    reg_la1_oenb = reg_la1_iena = 0xFFFFFFFF;
    reg_la1_oenb = reg_la1_iena = 0x0;
}
// ============================================================================


// ============================================================================
// MAIN ROUTINE
// ============================================================================
/*
 * main()
 * ----------------------------------------------------------------------------
 * Wakey Wakey Setup Protocol
 *  1. Configure Wake Output Pin IO_OUT[37]
 *  2. Configure PDM Data Input Pin IO_IN[36]
 *  3. Configure PDM Clock Output Pin IO_OUT[35]
 *  4. Configure PDM Activate Input Pin IO_IN[34]
 *  5. Write CFG Data via wishbone
 *  6. Read and Verify CFG Data via wishbone
 */

void main()
{
    // 1. Configure Wake Output Pin IO_OUT[37]
    reg_mprj_io_37 = GPIO_MODE_USER_STD_OUTPUT;

    // 2. Configure PDM Data Input Pin IO_IN[36]
    reg_mprj_io_36 = GPIO_MODE_USER_STD_INPUT_NOPULL;

    // 3. Configure PDM Clock Output Pin IO_OUT[35]
    reg_mprj_io_35 = GPIO_MODE_USER_STD_OUTPUT;

    // 4. Configure PDM Activate Input Pin IO_IN[34]
    reg_mprj_io_34 = GPIO_MODE_USER_STD_INPUT_PULLUP;

    reg_mprj_io_6 = 0x7ff;

    reg_mprj_datal = 0;

    reg_uart_clkdiv = 1042;
    reg_uart_enable = 1;

    reg_mprj_xfer = 1;
    while (reg_mprj_xfer == 1);

	// Enable GPIO (all output, ena = 0)
    reg_gpio_ena = 0x0;
    reg_gpio_pu = 0x0;
    reg_gpio_pd = 0x0;
    reg_gpio_data = 0x1;

    reg_la0_oenb = reg_la0_iena = 0xFFFFFFFF;
    reg_la1_oenb = reg_la1_iena = 0xFFFFFFFF;
    reg_la2_oenb = reg_la2_iena = 0xFFFFFFFF;
    reg_la3_oenb = reg_la3_iena = 0xFFFFFFFF;

    // reg_la0_data = 0x00000000;
    reg_la0_data = 0x00000001; //  [31:00] - writing 1 to [0] causes C1/C2 fail, enables ctl_pipeline_en
    reg_la1_data = 0x00000000; //  [63:32]
    reg_la2_data = 0x00000000; //  [95:64]
    reg_la3_data = 0x00400000; // [127:96]
    // wake is     0x00400000 (3rd bit is 118)

    reg_la0_oenb = reg_la0_iena = 0xFFFFFFFE; //  [31:00], enable [0] for ctl_pipeline_en
    reg_la1_oenb = reg_la1_iena = 0xFFFFFFFF; //  [63:32]
    reg_la2_oenb = reg_la2_iena = 0xFFFFFFFF; //  [95:64]
    reg_la3_oenb = reg_la3_iena = 0xFFBFFFFF; // [127:96], enable [117]

    // sleep until LCD boots up
    for (int i = 0; i < 20000; i++);

    // clear screen
    print("|"); putchar(0x2d); 

    // test conv1 mem
    print("CONV1 MEM: ");
    if (test_conv1_mem()) {
        print("PASS");
    } else {
        print("FAIL");
    }
    print("\n");

    // test conv2 mem
    print("CONV2 MEM: ");
    if (test_conv2_mem()) {
        print("PASS");
    } else {
        print("FAIL");
    }
    print("\n");

    // test fc mem
    print("   FC MEM: ");
    if (test_fc_mem()) {
        print("PASS");
    } else {
        print("FAIL");
    }

    while (1) {
        // toggle LED!
        reg_gpio_data = 0x1;
        for (int i = 0; i < 20000; i++);
        reg_gpio_data = 0x1;
        for (int i = 0; i < 20000; i++);
    }
}
// ============================================================================
