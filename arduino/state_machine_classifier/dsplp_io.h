/* 
 * Pinout setup for the DSPLP rev. 1.1 board.
 * KTH, Royal Institute of Technology, Sweden
 * Emil Karlsson, 2025
 * 
 * Does not include libraries or other constants for
 * the code, simply the hardware pinout. 
 */

// --- I2S --- //
#define I2S_MCK_IO      18
#define I2S_BCK_IO      45
#define I2S_WS_IO       47
#define I2S_DO_IO       48
#define I2S_DI_IO       46

// --- I2C --- //
// ES8388
#define ES_ADDRESS      0x10    // 0x10 base address + address pin
#define ES_SDA_IO       14
#define ES_SCL_IO       13

// TMP102
#define TMP_ADDRESS     0x49    // 0x48 base address + address pin
#define TMP_SDA_IO      2
#define TMP_SCL_IO      1
#define TMP_ALERT_IO    3
#define TMP_REG_TEMP    0x00    // temperature register (read only)
#define TMP_REG_CONF    0x01    // configuration register (read/write)
#define TMP_REG_TL      0x02    // T_LOW reg (read/write)
#define TMP_REG_TH      0x03    // T_HIGH reg (read/write)

// --- Shift reg --- //
#define LED_SDA_IO      15
#define LED_STCP_IO     17  // STorage register Clock
#define LED_SHCP_IO     16  // SHift register Clock
#define LED_SDA_MASK    (1 << LED_SDA_IO)
#define LED_STCP_MASK   (1 << LED_STCP_IO)
#define LED_SHCP_MASK   (1 << LED_SHCP_IO)
// Bitmasks are created for low-level access of the GPIO pins

// --- Potentiometers --- //
#define POT0_IO 10
#define POT1_IO 9

// --- Buttons --- //
#define SWA_IO 38
#define SWB_IO 35 