#include <EEPROM.h>

void float2Bytes(float val,byte* bytes_array){
  // Create union of shared memory space
  union {
    float float_variable;
    byte temp_array[4];
  } u;
  // Overite bytes of union with float variable
  u.float_variable = val;
  // Assign bytes to input array
  memcpy(bytes_array, u.temp_array, 4);
}

//float bytes2Float(char snelheidArray[])
//{
//  float snelheid;
//
//    union u_tag {
//       byte b[4];
//       float fval;
//    } u;
//    
//    u.b[0] = snelheidArray[0];
//    u.b[1] = snelheidArray[1];
//    u.b[2] = snelheidArray[2];
//    u.b[3] = snelheidArray[3];
//    
//    snelheid = u.fval;
//    return snelheid;
//}
float readValue(int addr) {
  float x; 
  int c = 0;
   char reads[4];
        for (int i=addr; i<addr+4; i++) 
        {
          reads[c]=EEPROM.read(i);
          c++;
        }
         x = *(float *)&reads;
        return x;
}

void writeValue(int addr, float value) {
  byte bytes[4];
  int  d = 0;
  float2Bytes(value,&bytes[d]);
  for (int i=addr; i<addr+4; i++) 
  {
    EEPROM.write(i,bytes[d]);
    d++;
  }

}
