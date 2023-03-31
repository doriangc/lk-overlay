#include <platform/bcm28xx/cm.h>

void hdmi_init() {

  // What's the pixel rate? 148 MHz for 1080p60

  int pixel_rate = 148 * 1000 * 1000; // NOTE: This only works for 1080p60
  int hsm_rate = (pixel_rate / 100) * 101; // Set HSM (HDMI State Machine) rate to 1% quicker than pixel rate

  // Set pixel clock



  // power_up_usb();
  setup_pllh(148 * 1000 * 1000); // This is the pixel clock
  hvs_initialize();

  #define CM_HSMCTL (CM_BASE + 0x088)
  #define CM_HSMCTL_ENAB_SET                                 0x00000010
  #define CM_HSMCTL_KILL_SET                                 0x00000020
  #define CM_HSMDIV (CM_BASE + 0x08c)
  int hsm_src = 5;
  *REG32(CM_HSMCTL) = CM_PASSWORD | CM_HSMCTL_KILL_SET | hsm_src;
  *REG32(CM_HSMDIV) = CM_PASSWORD | 0x2000;
  *REG32(CM_HSMCTL) = CM_PASSWORD | CM_HSMCTL_ENAB_SET | hsm_src;
#define HDMI_TX_PHY_TX_PHY_RESET_CTL 0x7e9022c0
  *REG32(HDMI_TX_PHY_TX_PHY_RESET_CTL) = 0xf << 16;
  *REG32(HDMI_TX_PHY_TX_PHY_RESET_CTL) = 0;
#define HDMI_SCHEDULER_CONTROL       0x7e9020c0
#define HDMI_FIFO_CTL                0x7e90205c
#define HDMI_VID_CTL                 0x7e902038
#define HDMI_RAM_PACKET_CONFIG       0x7e9020a0

// 	HDMI_WRITE(HDMI_SCHEDULER_CONTROL, HDMI_READ(HDMI_SCHEDULER_CONTROL) | VC4_HDMI_SCHEDULER_CONTROL_MANUAL_FORMAT | VC4_HDMI_SCHEDULER_CONTROL_IGNORE_VSYNC_PREDICTS);
  *REG32(HDMI_SCHEDULER_CONTROL) |= HDMI_SCHEDULER_CONTROL_MANUAL_FORMAT_SET | HDMI_SCHEDULER_CONTROL_IGN_VSYNC_PREDS_SET;

  struct pv_timings t;
  t.clock_mux = clk_dpi_smi_hdmi; // set

  t.vfp = 4; // Vertical front porch
  t.vsync = 5; // vertical sync time
  t.vbp = 36; // Vertical blanking period (16 pixels)
  t.vactive = 1920;

  t.vfp_even = 4; // try 3
  t.vsync_even = 3; // try 4
  t.vbp_even = 16;
  t.vactive_even = 1920;

  t.interlaced = false; // SET

  t.hfp = 88; // Horizontal front porch
  t.hsync = 44; // horizontal sync time (44 pixels)
  t.hbp = 148;
  t.hactive = 1080; 

  setup_pixelvalve(&t, 2);

  *REG32(HDMI_FIFO_CTL) = 1;

  #define VC4_HD_VID_CTL_ENABLE			BV(31)
  #define VC4_HD_VID_CTL_UNDERFLOW_ENABLE	BV(30)
  #define VC4_HD_VID_CTL_FRAME_COUNTER_RESET	BV(29)
  #define VC4_HD_VID_CTL_VSYNC_LOW		BV(28)
  #define VC4_HD_VID_CTL_HSYNC_LOW		BV(27)
  #define VC4_HD_VID_CTL_CLRSYNC			BV(24)
  #define VC4_HD_VID_CTL_CLRRGB			BV(23)
  #define VC4_HD_VID_CTL_BLANKPIX		BV(18)

  *REG32(HDMI_VID_CTL) = VC4_HD_VID_CTL_ENABLE | VC4_HD_VID_CTL_CLRRGB |VC4_HD_VID_CTL_UNDERFLOW_ENABLE |
		   VC4_HD_VID_CTL_FRAME_COUNTER_RESET |
		   (vsync_pos ? 0 : VC4_HD_VID_CTL_VSYNC_LOW) |
		   (hsync_pos ? 0 : VC4_HD_VID_CTL_HSYNC_LOW);

  *(HDMI_VID_CTL) &= ~VC4_HD_VID_CTL_BLANKPIX;

  #define VC4_HDMI_SCHEDULER_CONTROL_MANUAL_FORMAT BV(15)
  #define VC4_HDMI_SCHEDULER_CONTROL_IGNORE_VSYNC_PREDICTS BV(5)
  #define VC4_HDMI_SCHEDULER_CONTROL_VERT_ALWAYS_KEEPOUT	BV(3)
  #define VC4_HDMI_SCHEDULER_CONTROL_HDMI_ACTIVE	BV(1)
  #define VC4_HDMI_SCHEDULER_CONTROL_MODE_HDMI	BV(0)

  *(HDMI_SCHEDULER_CONTROL) |= VC4_HDMI_SCHEDULER_CONTROL_MODE_HDMI;

  while (*(HDMI_SCHEDULER_CONTROL) != VC4_HDMI_SCHEDULER_CONTROL_HDMI_ACTIVE) { udelay(1000 * 1000); }

  #define VC4_HDMI_RAM_PACKET_ENABLE  BV(16)
  *(HDMI_RAM_PACKET_CONFIG) = VC4_HDMI_RAM_PACKET_ENABLE;

  // cmd_gpio_dump_state();
}
