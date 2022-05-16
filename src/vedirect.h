
#ifndef VEDIRECT_H_
#define VEDIRECT_H_

#include <cstdint>
#include <string>

class VeDirect {
    private:
        // Serial Port to use, opend by fopen
        int SerialPort = -1;

    public:
        VeDirect();
        virtual ~VeDirect();

        void begin(string pathToSerialPort);
        int32_t read(uint8_t dest);
};

#endif /* VEDIRECT_H_ */
