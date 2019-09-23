
#include <lcom/lcf.h>
#include "RTC.h"

int rtchook = 8;

int RTC_Update_Check() {

	uint32_t byte = 0;

	sys_outb(RTC_ADDR_REG, RTC_REG_A);
	sys_inb(RTC_DATA_REG, &byte);

	return (byte & REG_A_UIP);
}

int RTC_Check_BCD() {

	uint32_t byte = 0;

	sys_outb(RTC_ADDR_REG, RTC_REG_B);
	sys_inb(RTC_DATA_REG, &byte);

	if (!(byte & REG_B_BCD))
		return 1;
		
	else
		return 0;

}


int decimal (unsigned long bcd)
{
	return (10*((bcd & 0xF0) >> 4)) + (bcd & 0x0F);
}

int rtc_subscribe()
{
	int bit_no=rtchook;
	
	if(sys_irqsetpolicy(IRQ_RTC, IRQ_REENABLE|IRQ_EXCLUSIVE,&bit_no)!=OK){
		return -1;
	}
	
	if(sys_irqenable(&bit_no)!=OK){
		return -2;
	}
	
	return BIT(rtchook);
}

int rtc_unsubscribe()
{
	int bit_no = rtchook;
	if(sys_irqrmpolicy(&bit_no)!=OK){
		return 1;
	}
	if(sys_irqenable(&bit_no)!=OK){
		return 1;
	}
	return 0;
}

void rtc_ih()
{
	int cause;
	uint32_t regA;

	sys_outb(RTC_ADDR_REG, RTC_REG_C);
	cause = sys_inb(RTC_DATA_REG, &regA);

	if(cause & REG_C_PF)
	{

	}
}

void set_rtc_periodic() 
{
	uint32_t byte;
	sys_outb(RTC_ADDR_REG, RTC_REG_B);
	sys_inb(RTC_DATA_REG, &byte);
	byte |= REG_B_PERIODIC|REG_B_HOUR;
	sys_outb(RTC_ADDR_REG, RTC_REG_B);
	sys_outb(RTC_DATA_REG, byte);
}

void set_rtc_alarm() 
{
	uint32_t byte;
	sys_outb(RTC_ADDR_REG, RTC_REG_B);
	sys_inb(RTC_DATA_REG, &byte);
	byte |=	REG_B_ALARM|REG_B_HOUR; 
	sys_outb(RTC_ADDR_REG, RTC_REG_B);
	sys_outb(RTC_DATA_REG, byte);

}

void set_rtc_update() 
{
	uint32_t byte;
	sys_outb(RTC_ADDR_REG, RTC_REG_B);
	sys_inb(RTC_DATA_REG, &byte);
	byte |= REG_B_UPDATE|REG_B_HOUR;
	sys_outb(RTC_ADDR_REG, RTC_REG_B);
	sys_outb(RTC_DATA_REG, byte);

}

int get_month() 
{
	uint32_t byte = 0;
	sys_outb(RTC_ADDR_REG, MONTH);
	sys_inb(RTC_DATA_REG, &byte);
	return decimal(byte);
}

int get_year() 
{
	uint32_t byte = 0;
	sys_outb(RTC_ADDR_REG, YEAR);
	sys_inb(RTC_DATA_REG, &byte);
	return decimal(byte);
}

int get_day() 
{
	uint32_t byte = 0;
	sys_outb(RTC_ADDR_REG, DAY);
	sys_inb(RTC_DATA_REG, &byte);
	return decimal(byte);
}

int get_hour() 
{
	uint32_t byte = 0;
	sys_outb(RTC_ADDR_REG, HOUR);
	sys_inb(RTC_DATA_REG, &byte);
	return decimal(byte);
}


int get_min() 
{
	uint32_t byte = 0;
	sys_outb(RTC_ADDR_REG, MINT);
	sys_inb(RTC_DATA_REG, &byte);
	return decimal(byte);
}

int get_sec()
{
	uint32_t byte = 0;
	sys_outb(RTC_ADDR_REG, SEC);
	sys_inb(RTC_DATA_REG, &byte);
	return decimal(byte);
}


