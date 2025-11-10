/** @file
 * SRC4392 driver.
 * @addtogroup SRC4392
 * @{
 */
#pragma once

namespace src4392 { 
    
/** SRC4392 driver class.
 */     
class Src4392 { 
    Src4392(Src4392 &&) = delete;
public:
    /** Initialize the SRC4392 device.
     * @return true on success, false on failure                    
     * 
     * This function initializes the SRC4392 device connected to the I2C bus. It configures the necessary registers to set up the device for operation.
     */
    bool init();        


};

} //!@} namespace
