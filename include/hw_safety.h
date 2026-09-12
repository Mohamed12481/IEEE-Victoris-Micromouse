#ifndef SAFETY_H
#define SAFETY_H

// ==========================================================
// No emergency-stop circuit is present in the hardware schematic.
// ==========================================================

// The schematic has no emergency-stop circuit, so this configures no GPIO.
void safety_init();

// Always false while no hardware emergency-stop input is installed.
bool safety_is_stopped();

#endif 
