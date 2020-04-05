#define RLEDs ((volatile long *) 0xFF200000)
volatile int pixel_buffer_start; // global variable
int main() {
	unsigned char byte1 = 0;
	unsigned char byte2 = 0;
	unsigned char byte3 = 0;
	
  	volatile int * PS2_ptr = (int *) 0xFF200100;  // PS/2 port address

	int PS2_data, RVALID;
	
	volatile int * pixel_ctrl_ptr = (int *)0xFF203020;
	/* Read location of the pixel buffer from the pixel buffer controller */
    pixel_buffer_start = *pixel_ctrl_ptr;
	clear_screen();
    int x0=0;
    int x1=319;
    int y0=0;
    int y1=0;
    int reverse=1;
	
	
	while (1) {
		
		draw_line(x0,y0,x1,y1,0x001F);
		wait_for_vsync();
		draw_line(x0,y0,x1,y1,0x000);
		PS2_data = *(PS2_ptr);	// read the Data register in the PS/2 port
		RVALID = (PS2_data & 0x8000);	// extract the RVALID field
		if (RVALID != 0)
		{
			/* al
			eceived */
			byte1 = byte2;
			byte2 = byte3;
			byte3 = PS2_data & 0xFF;
		}
		if ( (byte2 == 0xAA) && (byte3 == 0x00) )
		{
			// mouse inserted; initialize sending of data
			*(PS2_ptr) = 0xF4;
		}
		// Display last byte on Red LEDs
		if(byte3 == 0x74)
		{
			*RLEDs = 0x1;
			reverse =1;
		}
		if(byte3 == 0x6B)
		{
			*RLEDs = 0x2;
			reverse =-1;
		}
		if(byte3 == 0x76)
		{
			*RLEDs = 0x0;
			reverse = 0;
			
		}
		if(y0 == 0){reverse =1;}
		if(y1 == 239){reverse =-1;}
		y0+= reverse;
		y1+= reverse;
		pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer
	}
	return 0;
}

void wait_for_vsync(){

    volatile int * pixel_ctrl_ptr = 0xFF203020; // pixel controller
    register int status;
    *pixel_ctrl_ptr = 1; //start the synchronization process
    //wait for S to become 0
    status=*(pixel_ctrl_ptr +3);
    while((status & 0x01) != 0) {	
	status = *(pixel_ctrl_ptr +3); 
    }
}

void plot_pixel(int x, int y, short int line_color)
{
    *(short int *)(pixel_buffer_start + (y << 10) + (x << 1)) = line_color;
}

void clear_screen(){
	//draw black
	for (int x = 0; x < 320; x++){
		for (int y = 0; y < 240; y++){
	    		
			plot_pixel(x, y, 0x0000);	

		}
	}
}

//make the input valid for Bresenham's Algorithm
//x, y start from small values
void swap(int * x, int * y){
    int temp = *x;
    *x = *y;
    *y = temp;   
}


void draw_line(int x0, int y0, int x1, int y1, short int color) {
    
	//boolean is_steep = abs(y1 - y0) > abs(x1 - x0)
	int is_steep = 0;
    	int abs_y = y1 - y0;
    	int abs_x = x1 - x0;
    
    	if (abs_y < 0 ) abs_y = -abs_y;
    	if (abs_x < 0) abs_x = -abs_x;
    
    	if (abs_y > abs_x) is_steep=1;    
    	
	if(is_steep){
           swap(&x0, &y0);
           swap(&x1, &y1);
    	}
   
    	if(x0>x1){
           swap(&x0, &x1);
           swap(&y0, &y1);
    	}
    
    	int deltax = x1 - x0;
    	//int deltay = abs(y1 - y0)
	int deltay = y1-y0;
    	if(deltay <0) deltay = -deltay;
    	int error = -(deltax / 2);
    	int y = y0;
    	int y_step;
        
    	if(y0 < y1) y_step =1;
    	else y_step = -1;
    
    	for(int x=x0; x<=x1; x++) {
           
	   if (is_steep) plot_pixel(y,x, color);
           else plot_pixel(x,y, color);
        
           error = error + deltay;
        
           if(error>=0) {
            y +=y_step;
            error = error - deltax;
        }
    } 
}


