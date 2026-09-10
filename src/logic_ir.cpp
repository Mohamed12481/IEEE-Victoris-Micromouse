#include "logic_ir.h"

int cancel_ambient_light(int on_reading, int ambient_reading) {
    int diff = on_reading - ambient_reading;
    return (diff < 0) ? 0 : diff;
}