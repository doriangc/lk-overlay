#include <platform.h>
#include <platform/bcm28xx/cm.h>
#include <platform/bcm28xx/pll.h>
#include <platform/bcm28xx/hvs.h>
#include <platform/bcm28xx/pv.h>
#include <platform/bcm28xx/udelay.h>
#include <platform/bcm28xx/power.h>
#include <platform/bcm28xx/a2w.h>
#include <lk/reg.h>
#include <lk/init.h>


static void hdmi_init(uint level) {

  // What's the pixel rate? 148 MHz for 1080p60
  int pixel_rate = 1485 * 100 * 1000; // NOTE: This only works for 1080p60 (see https://tomverbeure.github.io/video_timings_calculator)
  int hsm_rate = (pixel_rate / 100) * 101; // Set HSM (HDMI State Machine) rate to 1% quicker than pixel rate.

  printf("1: %X\n", *REG32(PM_HDMI));
    // Turn on HDMI power
  #define PM_HDMI_RSTDR (1 << 19)
  #define PM_HDMI_CTRLEN 1
  #define PM_HDMI_LDOPD (1 << 1)

  *REG32(PM_HDMI) |= PM_PASSWORD | PM_HDMI_RSTDR;
  *REG32(PM_HDMI) |= PM_PASSWORD | PM_HDMI_CTRLEN;
  *REG32(PM_HDMI) = PM_PASSWORD | (*REG32(PM_HDMI) & ~PM_HDMI_LDOPD);
  // usleep_range(100, 200);
  udelay(150);
  *REG32(PM_HDMI) = PM_PASSWORD | (*REG32(PM_HDMI) & ~PM_HDMI_RSTDR);
  udelay(1000*1000);

  printf("1: %X\n", *REG32(PM_HDMI));

  // power_up_usb();
  // setup_pllh(pixel_rate*10); // Set the pixel clock

  // #define A2W_XOCS_CTRL (A2W_BASE + 0x190)
	*REG32(A2W_XOSC_CTRL) |= 1 | CM_PASSWORD; // Enable XOCS PLCC (PLLH's domain)

  printf("1b\n");

  uint32_t ana[4];
  uint32_t ndiv = 0x4d;
  uint32_t fdiv = 0x58000;

	ana[0] = 0x900000;
	ana[1] = 0xc;
	ana[2] = 0;
	ana[3] = 0;
	for (int i = 3; i >= 0; --i)
		*REG32(A2W_PLLH_ANA0 + i*4) = ana[i] | CM_PASSWORD;
  
   printf("1c\n");

  *REG32(A2W_PLLH_FRAC) = fdiv | CM_PASSWORD;
  printf("1d\n");

	uint32_t val = *REG32(A2W_PLLH_CTRL);
	val &= ~(((1ul << 10) - 1) << 0); // First 10 least significant bits (A2W_PLL_CTRL_NDIV)
	val &= ~(((1ul << 3) - 1) << 12);
	val |= (((ndiv) & ((1ul << 10) - 1)) << 0);
	val |= (((1) & ((1ul << 3) - 1)) << 12);
	*REG32(A2W_PLLH_CTRL) = val | CM_PASSWORD;

  printf("1e\n");

	// Set PLLH_PIX Parameters.
	val = *REG32(A2W_PLLH_PIX);
	val &= ~(((1ul << 8) - 1) << 0);
	val |= (((0x1) & ((1ul << 8) - 1)) << 0);
	*REG32(A2W_PLLH_PIX) = val | CM_PASSWORD;

  printf("1f\n");

	// Reload PLLH_PIX Parameters.
	val = *REG32(CM_PLLH);
	*REG32(CM_PLLH) = val | (((1ul << 1) - 1) << 0) | CM_PASSWORD;
	*REG32(CM_PLLH) = (val & ~(((1ul << 1) - 1) << 0)) | CM_PASSWORD;

  printf("1g\n");

	// Enable PLLH.
	val = *REG32(A2W_PLLH_CTRL);
	val &= ~(((1ul << 1) - 1) << 16);
	*REG32(A2W_PLLH_CTRL) = val | CM_PASSWORD;

  printf("1h\n");


	val = *REG32(CM_PLLH);
	val &= ~(((1ul << 1) - 1) << 8);
	*REG32(CM_PLLH) = val | CM_PASSWORD;
  printf("1i\n");


	while (!(0x404 & *REG32(CM_LOCK)))
		;
  
  printf("1j\n");


	val = (((1ul << 1) - 1) << 17);
	*REG32(A2W_PLLH_CTRL) |= val | CM_PASSWORD;

  printf("1k\n");


	// Enable PLLH_PIXe.
	val = *REG32(A2W_PLLH_PIX);
	val &= ~(((1ul << 1) - 1) << 8);
	*REG32(A2W_PLLH_PIX) = val | CM_PASSWORD;

  printf("1l\n");

  // udelay(1000*1000);
  printf("2");
  hvs_initialize();
  udelay(1000*1000);
  // printf("3");

  // This is a clock (HSM = HDMI State Machine)
  #define CM_HSMCTL (CM_BASE + 0x088)
  #define CM_HSMCTL_ENAB_SET                                 0x00000010
  #define CM_HSMCTL_KILL_SET                                 0x00000020
  #define CM_HSMDIV (CM_BASE + 0x08c)

  int hsm_src = 5; // PLLC. it depends on current ARM frequency but it *should* be set to 486 Mhz (Actually might be 972 through doubler)
  udelay(1000*1000);
  printf("4");
  *REG32(CM_HSMCTL) = CM_PASSWORD | CM_HSMCTL_KILL_SET | hsm_src;
  udelay(1000*1000);
  printf("5");
  *REG32(CM_HSMDIV) = CM_PASSWORD | (6 << 12); // A divisor of 3 on 486 Mhz is about 162 Mhz, or slightly faster than the 148.5 Mhz pixel clock
  udelay(1000*1000);
  printf("6");
  *REG32(CM_HSMCTL) = CM_PASSWORD | CM_HSMCTL_ENAB_SET | hsm_src;
  udelay(1000*1000);


  #define HDMI_TX_PHY_TX_PHY_RESET_CTL 0x7e9022c0
                                      //  0x7e902000
  #define HDMI_VID_CTL                 0x7e808038 // HD register
  #define HDMI_M_CTL                   0x7e80800c // HD register
  #define HDMI_SW_RESET_CONTROL        0x7e902004 

  printf("7");
  // int rev = *REG32(0x7e902000);
  // printf("HDMI CORE REVISION %X\n", rev);
  
  // *REG32(HDMI_M_CTL) = 1 << 2;
	// udelay(1);
	// *REG32(HDMI_M_CTL) = 0;
  // printf("AA\n");

	// *REG32(HDMI_M_CTL) = 1;
  // printf("AB\n");


	// *REG32(HDMI_SW_RESET_CONTROL) = 3;
  // printf("AC\n");

	// *REG32(HDMI_SW_RESET_CONTROL) = 0;
  // udelay(1000*1000);

  printf("7b");

  // *REG32(HDMI_VID_CTL) |= 1 << 18;
  *REG32(HDMI_TX_PHY_TX_PHY_RESET_CTL) = 0xf << 16; // 
  udelay(1000*1000);
  printf("8");
  *REG32(HDMI_TX_PHY_TX_PHY_RESET_CTL) = 0;
  udelay(1000*1000);
  printf("9");

  // return;

#define HDMI_SCHEDULER_CONTROL       0x7e9020c0
#define HDMI_FIFO_CTL                0x7e90205c

#define HDMI_CSC_CTL                 0x7e902040
#define HDMI_RAM_PACKET_CONFIG       0x7e9020a0

#define HDMI_SCHEDULER_CONTROL_MANUAL_FORMAT_SET 1 << 15
#define HDMI_SCHEDULER_CONTROL_IGN_VSYNC_PREDS_SET 1 << 5

// 	HDMI_WRITE(HDMI_SCHEDULER_CONTROL, HDMI_READ(HDMI_SCHEDULER_CONTROL) | VC4_HDMI_SCHEDULER_CONTROL_MANUAL_FORMAT | VC4_HDMI_SCHEDULER_CONTROL_IGNORE_VSYNC_PREDICTS);
  *REG32(HDMI_SCHEDULER_CONTROL) |= HDMI_SCHEDULER_CONTROL_MANUAL_FORMAT_SET | HDMI_SCHEDULER_CONTROL_IGN_VSYNC_PREDS_SET;
  udelay(1000*1000);
  printf("10");


  struct pv_timings t;
  t.clock_mux = clk_dpi_smi_hdmi; // set clock to HDMI clock

  // https://tomverbeure.github.io/video_timings_calculator (CEA-861)
  t.hfp = 88; // Horizontal front porcht
  t.hsync = 44; // horizontal sync time (44 pixels)
  t.hbp = 148;
  t.hactive = 1920;

  t.vfp = 4; // Vertical front porch
  t.vsync = 5; // vertical sync time
  t.vbp = 36; // Vertical blanking period (16 pixels)
  t.vactive = 1080;

  // t.vfp_even = 4; // try 3
  // t.vsync_even = 3; // try 4
  // t.vbp_even = 16;
  // t.vactive_even = 1920;


  t.interlaced = false; // SET
  udelay(1000*1000);
  printf("11");

  setup_pixelvalve(&t, 2); // PixelValve 2 controls both HDMI and the Composite port on the Raspberry Pi 0-3
  udelay(1000*1000);
  printf("12");

  // This is related to setting the RGB mode, we are operating in RGB full here.
  *REG32(HDMI_CSC_CTL) = 1 << 5; // VC4_HD_CSC_CTL_ORDER_BGR
  udelay(1000*1000);
  printf("13");

  *REG32(HDMI_FIFO_CTL) = 1; // VC4_HDMI_FIFO_CTL_MASTER_SLAVE_N
  udelay(1000*1000);
  printf("14");

  #define VC4_HD_VID_CTL_ENABLE			BV(31)
  #define VC4_HD_VID_CTL_UNDERFLOW_ENABLE	BV(30)
  #define VC4_HD_VID_CTL_FRAME_COUNTER_RESET	BV(29)
  #define VC4_HD_VID_CTL_VSYNC_LOW		BV(28)
  #define VC4_HD_VID_CTL_HSYNC_LOW		BV(27)
  #define VC4_HD_VID_CTL_CLRSYNC			BV(24)
  #define VC4_HD_VID_CTL_CLRRGB			BV(23)
  #define VC4_HD_VID_CTL_BLANKPIX		BV(18)

  bool vsync_pos = true; // assume horizontal sync is active high. Not sure this is right, but it should be.
  bool hsync_pos = true;
  
  *REG32(HDMI_VID_CTL) = VC4_HD_VID_CTL_ENABLE | VC4_HD_VID_CTL_CLRRGB | VC4_HD_VID_CTL_UNDERFLOW_ENABLE |
		   VC4_HD_VID_CTL_FRAME_COUNTER_RESET |
		   (vsync_pos ? 0 : VC4_HD_VID_CTL_VSYNC_LOW) |
		   (hsync_pos ? 0 : VC4_HD_VID_CTL_HSYNC_LOW);
  
  udelay(1000*1000);
  printf("15");

  *REG32(HDMI_VID_CTL) &= ~VC4_HD_VID_CTL_BLANKPIX;
  
  udelay(1000*1000);
  printf("16");

  #define VC4_HDMI_SCHEDULER_CONTROL_MANUAL_FORMAT BV(15)
  #define VC4_HDMI_SCHEDULER_CONTROL_IGNORE_VSYNC_PREDICTS BV(5)
  #define VC4_HDMI_SCHEDULER_CONTROL_VERT_ALWAYS_KEEPOUT	BV(3)
  #define VC4_HDMI_SCHEDULER_CONTROL_HDMI_ACTIVE	BV(1)
  #define VC4_HDMI_SCHEDULER_CONTROL_MODE_HDMI	BV(0)

  *REG32(HDMI_SCHEDULER_CONTROL) |= VC4_HDMI_SCHEDULER_CONTROL_MODE_HDMI;
  udelay(1000*1000);
  printf("17");


  uint i=0;
  while (*REG32(HDMI_SCHEDULER_CONTROL) != VC4_HDMI_SCHEDULER_CONTROL_HDMI_ACTIVE && i < 10) { udelay(1000 * 1000); i++; }
  udelay(1000*1000);
  printf("18");

  if (*REG32(HDMI_SCHEDULER_CONTROL) != VC4_HDMI_SCHEDULER_CONTROL_HDMI_ACTIVE) {
    // Timeout
    return;
  }

  udelay(1000*1000);
  printf("19");


  #define VC4_HDMI_RAM_PACKET_ENABLE  BV(16)
  *REG32(HDMI_RAM_PACKET_CONFIG) = VC4_HDMI_RAM_PACKET_ENABLE;
  udelay(1000*1000);
  printf("20");

  // May need some infoframe stuff here.

  // cmd_gpio_dump_state();
}

LK_INIT_HOOK(vec, &hdmi_init, LK_INIT_LEVEL_PLATFORM - 1);