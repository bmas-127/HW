#include "mbed.h"
#include "fsl_port.h"
#include "fsl_gpio.h"
#include <iostream>

#define UINT14_MAX        16383

// FXOS8700CQ I2C address
#define FXOS8700CQ_SLAVE_ADDR0 (0x1E<<1) // with pins SA0=0, SA1=0
#define FXOS8700CQ_SLAVE_ADDR1 (0x1D<<1) // with pins SA0=1, SA1=0
#define FXOS8700CQ_SLAVE_ADDR2 (0x1C<<1) // with pins SA0=0, SA1=1
#define FXOS8700CQ_SLAVE_ADDR3 (0x1F<<1) // with pins SA0=1, SA1=1

// FXOS8700CQ internal register addresses
#define FXOS8700Q_STATUS 0x00
#define FXOS8700Q_OUT_X_MSB 0x01
#define FXOS8700Q_OUT_Y_MSB 0x03
#define FXOS8700Q_OUT_Z_MSB 0x05
#define FXOS8700Q_M_OUT_X_MSB 0x33
#define FXOS8700Q_M_OUT_Y_MSB 0x35
#define FXOS8700Q_M_OUT_Z_MSB 0x37
#define FXOS8700Q_WHOAMI 0x0D
#define FXOS8700Q_XYZ_DATA_CFG 0x0E
#define FXOS8700Q_CTRL_REG1 0x2A
#define FXOS8700Q_M_CTRL_REG1 0x5B
#define FXOS8700Q_M_CTRL_REG2 0x5C
#define FXOS8700Q_WHOAMI_VAL 0xC7

I2C i2c( PTD9,PTD8);
Serial pc(USBTX, USBRX);

DigitalOut led(LED3);
DigitalOut led2(LED1);
InterruptIn sw2(SW2);
EventQueue queue;
Thread thread;
Timer debounce;

int m_addr = FXOS8700CQ_SLAVE_ADDR1;

void FXOS8700CQ_readRegs(int addr, uint8_t * data, int len);
void FXOS8700CQ_writeRegs(uint8_t * data, int len);
void sample();

float ref_vec[4];

int main() {
   uint8_t *data, *res;
   uint8_t who_am_i;
   int16_t acc16;

   data = new uint8_t [2];
   res = new uint8_t [6];
   led = 1;
   led2 = 1;

   // Enable the FXOS8700Q
   FXOS8700CQ_readRegs( FXOS8700Q_CTRL_REG1, &data[1], 1);

   data[1] |= 0x01;
   data[0] = FXOS8700Q_CTRL_REG1;
   FXOS8700CQ_writeRegs(data, 2);

   // Get the slave address
   FXOS8700CQ_readRegs(FXOS8700Q_WHOAMI, &who_am_i, 1);

   // record initial state
   for(int i = 0; i < 10; i ++ )
      FXOS8700CQ_readRegs(FXOS8700Q_OUT_X_MSB, res, 6);

      acc16 = (res[0] << 6) | (res[1] >> 2);
      if (acc16 > UINT14_MAX/2)
         acc16 -= UINT14_MAX;
      ref_vec[0] = ((float)acc16) / 4096.0f;

      acc16 = (res[2] << 6) | (res[3] >> 2);
      if (acc16 > UINT14_MAX/2)
         acc16 -= UINT14_MAX;
      ref_vec[1] = ((float)acc16) / 4096.0f;

      acc16 = (res[4] << 6) | (res[5] >> 2);
      if (acc16 > UINT14_MAX/2)
         acc16 -= UINT14_MAX;
      ref_vec[2] = ((float)acc16) / 4096.0f;

      ref_vec[3] = ref_vec[0] * ref_vec[0] + ref_vec[1] * ref_vec[1] + ref_vec[2] * ref_vec[2];

   // thread init and interupt signal control
   debounce.start();
   thread.start(callback(&queue, &EventQueue::dispatch_forever));
   sw2.rise(queue.event(sample));

//   pc.printf("Here is %x\r\n", who_am_i);

   while(1);

}

void sample(){
   uint8_t res[6];
   int16_t acc16;
   float t[3];

   if(debounce.read_ms() > 1000){
      debounce.reset();
   }else{
      return;
   }
   
   for(int i = 0; i < 120; i ++ ){
      float angle;

      led = !led;
      FXOS8700CQ_readRegs(FXOS8700Q_OUT_X_MSB, res, 6);

      acc16 = (res[0] << 6) | (res[1] >> 2);
      if (acc16 > UINT14_MAX/2)
         acc16 -= UINT14_MAX;
      t[0] = ((float)acc16) / 4096.0f;

      acc16 = (res[2] << 6) | (res[3] >> 2);
      if (acc16 > UINT14_MAX/2)
         acc16 -= UINT14_MAX;
      t[1] = ((float)acc16) / 4096.0f;

      acc16 = (res[4] << 6) | (res[5] >> 2);
      if (acc16 > UINT14_MAX/2)
         acc16 -= UINT14_MAX;
      t[2] = ((float)acc16) / 4096.0f;


      angle = (t[0]*ref_vec[0] + t[1]*ref_vec[1] + t[2]*ref_vec[2]);
      angle *= angle;
      angle /= ( (t[1]*t[1]+ t[0]*t[0]+t[2]*t[2]) * ref_vec[3]);
      angle = (angle < 0.5) ? 1 : 0;

      led2 = (angle == 1) ? 0 : 1;

      pc.printf("%1.4f\r\n%1.4f\r\n%1.4f\r\n",t[0], t[1], t[2]);
      pc.printf("%f\r\n", angle);
      wait(0.1);
   };
}


void FXOS8700CQ_readRegs(int addr, uint8_t * data, int len) {
   char t = addr;
   i2c.write(m_addr, &t, 1, true);
   i2c.read(m_addr, (char *)data, len);
}


void FXOS8700CQ_writeRegs(uint8_t * data, int len) {
   i2c.write(m_addr, (char *)data, len);
}
