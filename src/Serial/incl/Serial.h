#ifndef SERIAL_H
#define SERIAL_H

class Serial
{
public:
   
    static int init_serial();
    static int init_serial_simple();
    static int write_float(int port, float f);
    static int write_serial(int port, const char* text);
    static const char* read_serial(int port);
private:
    Serial();
};

#endif // SERIAL_H
