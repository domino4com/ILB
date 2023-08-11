#include "ILB.h"

ILB::ILB(int addr) {
    _wire = &Wire;
    i2cAddress = addr;
}

ILB::ILB() {
    _wire = &Wire;
    i2cAddress = ILB_ADDRESS;
}

ILB::ILB(TwoWire *w, int addr) {
    _wire = w;
    i2cAddress = addr;
}

ILB::ILB(TwoWire *w) {
    _wire = w;
    i2cAddress = ILB_ADDRESS;
}

bool ILB::begin() {
    uint8_t part_id = readRegister(ILB_PART_ID);

    if ((part_id >> 4) != 0x0B) {
        return false;
    }

    if (!reset()) {
        return false;
    }

    enable(true);
    if (!enabled()) {
        return false;
    }

    setGain(ILB_GAIN_3);
    setResolution(ILB_RESOLUTION_18BIT);

    return true;
}

bool ILB::getData(float &UVI, float &Lux) {
    setGain(ILB_GAIN_3);                  // Recommended for Lux - x3
    setResolution(ILB_RESOLUTION_18BIT);  // Recommended for Lux - 18-bit
    setMode(ILB_MODE_ALS);
    delay(100);
    while (!newDataAvailable())
        ;
    Lux = getLux();

    setGain(ILB_GAIN_18);                 // Recommended for UVI - x18
    setResolution(ILB_RESOLUTION_20BIT);  // Recommended for UVI - 20-bit
    setMode(ILB_MODE_UVS);
    delay(400);
    while (!newDataAvailable())
        ;
    UVI = getUVI();

    return true;  // Return true for successful read (add error handling if needed)
}

bool ILB::getJSON(JsonObject &doc) {
    float UVI, Lux;
    if (!getData(UVI, Lux)) {
        return false;
    }

    JsonArray dataArray = doc.createNestedArray("ILB");

    JsonObject dataSet = dataArray.createNestedObject();  // First data set
    dataSet["name"] = "UVI";
    dataSet["value"] = UVI;
    dataSet["unit"] = "";

    dataSet = dataArray.createNestedObject();   // Subsequent data sets
    dataSet["name"] = "Lux";
    dataSet["value"] = Lux;
    dataSet["unit"] = "lx";

    return true;
}

/*!
 *  @brief  Perform a soft reset with 10ms delay.
 *  @returns True on success (reset bit was cleared post-write)
 */
bool ILB::reset(void) {
    uint8_t _r = readRegister(ILB_MAIN_CTRL);
    _r |= B00010000;
    writeRegister(ILB_MAIN_CTRL, _r);
    delay(10);
    _r = readRegister(ILB_MAIN_CTRL);
    if (_r != 0) {
        return false;
    }

    return true;
}

/*!
 *  @brief  Checks if new data is available in data register
 *  @returns True on new data available
 */
bool ILB::newDataAvailable(void) {
    uint8_t status = readRegister(ILB_MAIN_STATUS);
    status >>= 3;
    status &= 1;
    return status;
}

/*!
 *  @brief  Read 3-bytes out of ambient data register, does not check if data is
 * new!
 *  @returns Up to 20 bits, right shifted into a 32 bit int
 */
uint32_t ILB::readALS(void) {
    uint8_t _lsb = readRegister(ILB_ALSDATA_LSB);
    uint8_t _msb = readRegister(ILB_ALSDATA_MSB);
    uint8_t _hsb = readRegister(ILB_ALSDATA_HSB);
    _hsb &= 0x0F;
    uint32_t _out = ((uint32_t)_hsb << 16) | ((uint16_t)_msb << 8) | _lsb;
    return _out;
}

/*!
 *  @brief  Read 3-bytes out of UV data register, does not check if data is new!
 *  @returns Up to 20 bits, right shifted into a 32 bit int
 */
uint32_t ILB::readUVS(void) {
    uint8_t _lsb = readRegister(ILB_UVSDATA_LSB);
    uint8_t _msb = readRegister(ILB_UVSDATA_MSB);
    uint8_t _hsb = readRegister(ILB_UVSDATA_HSB);
    _hsb &= 0x0F;
    uint32_t _out = ((uint32_t)_hsb << 16) | ((uint16_t)_msb << 8) | _lsb;
    return _out;
}

float ILB::getLux() {
    uint32_t raw = readALS();
    uint8_t _gain = (uint8_t)(getGain());
    uint8_t _resolution = (uint8_t)(getResolution());
    float lux = 0.6 * (float)(raw) / (gain_factor[_gain] * res_factor[_resolution]) * (float)(WFAC);
    return lux;
}

float ILB::getUVI() {
    uint32_t raw = readUVS();
    uint8_t _gain = (uint8_t)(getGain());
    uint8_t _resolution = (uint8_t)(getResolution());
    float uvi = (float)(raw) / ((gain_factor[_gain] / gain_factor[ILB_GAIN_18]) * (res_factor[_resolution] / res_factor[ILB_RESOLUTION_20BIT]) * (float)(UV_SENSITIVITY)) * (float)(WFAC);
    return uvi;
}

/*!
 *  @brief  Enable or disable the light sensor
 *  @param  en True to enable, False to disable
 */
void ILB::enable(bool en) {
    uint8_t _r = readRegister(ILB_MAIN_CTRL);
    _r |= (1 << 1);
    writeRegister(ILB_MAIN_CTRL, (uint8_t)_r);
}

/*!
 *  @brief  Read the enabled-bit from the sensor
 *  @returns True if enabled
 */
bool ILB::enabled(void) {
    uint8_t _r = readRegister(ILB_MAIN_CTRL);
    _r >>= 1;
    _r &= 1;
    return _r;
}

/*!
 *  @brief  Set the sensor mode to EITHER ambient (ILB_MODE_ALS) or UV
 * (ILB_MODE_UVS)
 *  @param  mode The desired mode - ILB_MODE_UVS or ILB_MODE_ALS
 */
void ILB::setMode(ILB_mode_t mode) {
    uint8_t _r = readRegister(ILB_MAIN_CTRL);
    _r &= 0xF7;
    _r |= ((uint8_t)mode << 3);
    writeRegister(ILB_MAIN_CTRL, (uint8_t)_r);
}

/*!
 *  @brief  get the sensor's mode
 *  @returns The current mode - ILB_MODE_UVS or ILB_MODE_ALS
 */
ILB_mode_t ILB::getMode(void) {
    uint8_t _r = readRegister(ILB_MAIN_CTRL);
    _r >>= 3;
    _r &= 1;
    return (ILB_mode_t)_r;
}

/*!
 *  @brief  Set the sensor gain
 *  @param  gain The desired gain: ILB_GAIN_1, ILB_GAIN_3, ILB_GAIN_6
 *  ILB_GAIN_9 or ILB_GAIN_18
 */
void ILB::setGain(ILB_gain_t gain) {
    writeRegister(ILB_GAIN, (uint8_t)gain);
}

/*!
 *  @brief  Get the sensor's gain
 *  @returns gain The current gain: ILB_GAIN_1, ILB_GAIN_3, ILB_GAIN_6
 *  ILB_GAIN_9 or ILB_GAIN_18
 */
ILB_gain_t ILB::getGain(void) {
    uint8_t _r = readRegister(ILB_GAIN);
    _r &= 7;
    return (ILB_gain_t)_r;
}

/*!
 *  @brief  Set the sensor resolution. Higher resolutions take longer to read!
 *  @param  res The desired resolution: ILB_RESOLUTION_13BIT,
 *  ILB_RESOLUTION_16BIT, ILB_RESOLUTION_17BIT, ILB_RESOLUTION_18BIT,
 *  ILB_RESOLUTION_19BIT or ILB_RESOLUTION_20BIT
 */
void ILB::setResolution(ILB_resolution_t res) {
    uint8_t _r = 0;
    _r |= (res << 4);
    writeRegister(ILB_MEAS_RATE, (uint8_t)_r);
}

/*!
 *  @brief  Get the sensor's resolution
 *  @returns The current resolution: ILB_RESOLUTION_13BIT,
 *  ILB_RESOLUTION_16BIT, ILB_RESOLUTION_17BIT, ILB_RESOLUTION_18BIT,
 *  ILB_RESOLUTION_19BIT or ILB_RESOLUTION_20BIT
 */
ILB_resolution_t ILB::getResolution(void) {
    uint8_t _r = readRegister(ILB_MEAS_RATE);
    _r &= 0x70;
    _r = 7 & (_r >> 4);
    return (ILB_resolution_t)_r;
}

/*!
 *  @brief  Set the interrupt output threshold range for lower and upper.
 *  When the sensor is below the lower, or above upper, interrupt will fire
 *  @param  lower The lower value to compare against the data register.
 *  @param  higher The higher value to compare against the data register.
 */
void ILB::setThresholds(uint32_t lower, uint32_t higher) {
    uint8_t _r = higher;
    writeRegister(ILB_THRESH_UP, _r);
    _r = higher >> 8;
    writeRegister(ILB_THRESH_UP + 1, _r);
    _r = higher >> 16;
    _r &= 0x0F;
    writeRegister(ILB_THRESH_UP + 2, _r);
    _r = lower;
    writeRegister(ILB_THRESH_LOW, _r);
    _r = lower >> 8;
    writeRegister(ILB_THRESH_LOW + 1, _r);
    _r = lower >> 16;
    _r &= 0x0F;
    writeRegister(ILB_THRESH_LOW + 2, _r);
}

/*!
 *  @brief  Configure the interrupt based on the thresholds in setThresholds()
 *  When the sensor is below the lower, or above upper thresh, interrupt will
 * fire
 *  @param  enable Whether the interrupt output is enabled
 *  @param  source Whether to use the ALS or UVS data register to compare
 *  @param  persistance The number of consecutive out-of-range readings before
 *          we fire the IRQ. Default is 0 (each reading will fire)
 */
void ILB::configInterrupt(bool enable, ILB_mode_t source,
                          uint8_t persistance) {
    uint8_t _r = 0;
    _r |= (enable << 2) | (1 << 4) | (source << 5);
    writeRegister(ILB_INT_CFG, _r);
    if (persistance > 0x0F) persistance = 0x0F;
    uint8_t _p = 0;
    _p |= persistance << 4;
    writeRegister(ILB_INT_PST, _p);
}

uint8_t ILB::writeRegister(uint8_t reg, uint8_t val) {
    _wire->beginTransmission(i2cAddress);
    _wire->write(reg);
    _wire->write(val);
    return _wire->endTransmission();
}

uint8_t ILB::readRegister(uint8_t reg) {
    uint8_t regValue = 0;
    _wire->beginTransmission(i2cAddress);
    _wire->write(reg);
    _wire->endTransmission();
    _wire->requestFrom(i2cAddress, 1);
    if (_wire->available()) {
        regValue = _wire->read();
    }
    return regValue;
}
