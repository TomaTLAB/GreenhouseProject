#include "UniversalSensors.h"
#include <OneWire.h>
#include <EEPROM.h>

UniRegDispatcher UniDispatcher;

UniRegDispatcher::UniRegDispatcher()
{
  mainController = NULL;
  temperatureModule = NULL;
  humidityModule = NULL;
  luminosityModule = NULL;
  soilMoistureModule = NULL;  

  currentTemperatureCount = 0;
  currentHumidityCount = 0;
  currentLuminosityCount = 0;
  currentSoilMoistureCount = 0;

  hardCodedTemperatureCount = 0;
  hardCodedHumidityCount = 0;
  hardCodedLuminosityCount = 0;
  hardCodedSoilMoistureCount = 0;
    
}
uint8_t UniRegDispatcher::GetHardCodedSensorsCount(UniSensorType type)
{
  switch(type)
  {
    case uniNone: return 0;
    case uniTemp: return hardCodedTemperatureCount;
    case uniHumidity: return hardCodedHumidityCount;
    case uniLuminosity: return hardCodedLuminosityCount;
    case uniSoilMoisture: return hardCodedSoilMoistureCount;
  }

  return 0;
}
void UniRegDispatcher::Setup(ModuleController* controller)
{
  mainController = controller;
  if(!mainController)
    return;

    temperatureModule = mainController->GetModuleByID(F("STATE"));
    if(temperatureModule)
      hardCodedTemperatureCount = temperatureModule->State.GetStateCount(StateTemperature);
    
    humidityModule = mainController->GetModuleByID(F("HUMIDITY"));
    if(humidityModule)
      hardCodedHumidityCount = humidityModule->State.GetStateCount(StateTemperature);
    
    luminosityModule = mainController->GetModuleByID(F("LIGHT"));
    if(luminosityModule)
      hardCodedLuminosityCount = luminosityModule->State.GetStateCount(StateLuminosity);

    soilMoistureModule = mainController->GetModuleByID(F("SOIL"));
    if(soilMoistureModule)
      hardCodedSoilMoistureCount = soilMoistureModule->State.GetStateCount(StateSoilMoisture);


    ReadState(); // читаем последнее запомненное состояние
    RestoreState(); // восстанавливаем последнее запомненное состояние
}
void UniRegDispatcher::ReadState()
{
  //Тут читаем последнее запомненное состояние по индексам сенсоров
  uint16_t addr = UNI_SENSOR_INDICIES_EEPROM_ADDR;
  uint8_t val = EEPROM.read(addr++);
  if(val != 0xFF)
    currentTemperatureCount = val;

  val = EEPROM.read(addr++);
  if(val != 0xFF)
    currentHumidityCount = val;

  val = EEPROM.read(addr++);
  if(val != 0xFF)
    currentLuminosityCount = val;

  val = EEPROM.read(addr++);
  if(val != 0xFF)
    currentSoilMoistureCount = val;
      
}
void UniRegDispatcher::RestoreState()
{
  //Тут восстанавливаем последнее запомненное состояние индексов сенсоров.
  // добавляем новые датчики в нужный модуль до тех пор, пока
  // их кол-во не сравняется с сохранённым последним выданным индексом.
  // индексы универсальным датчикам выдаются, начиная с 0, при этом данный индекс является
  // виртуальным, поэтому нам всегда надо добавить датчик в конец
  // списка, после жёстко указанных в прошивке датчиков. Такой подход
  // обеспечит нормальную работу универсальных датчиков вне зависимости
  // от настроек прошивки.
  
  if(temperatureModule)
  {
    uint8_t cntr = 0;    
    while(cntr < currentTemperatureCount)
    {
      temperatureModule->State.AddState(StateTemperature, hardCodedTemperatureCount + cntr);
      cntr++;
    }
    
  } // if(temperatureModule)

  if(humidityModule)
  {
    uint8_t cntr = 0;
    while(cntr < currentHumidityCount)
    {
      humidityModule->State.AddState(StateTemperature, hardCodedHumidityCount + cntr);
      humidityModule->State.AddState(StateHumidity, hardCodedHumidityCount + cntr);
      cntr++;
    }
    
  } // if(humidityModule)

 if(luminosityModule)
  {
    uint8_t cntr = 0;
    while(cntr < currentLuminosityCount)
    {
      luminosityModule->State.AddState(StateLuminosity, hardCodedLuminosityCount + cntr);
      cntr++;
    }
    
  } // if(luminosityModule)  

if(soilMoistureModule)
  {
    uint8_t cntr = 0;
    while(cntr < currentSoilMoistureCount)
    {
      soilMoistureModule->State.AddState(StateSoilMoisture, hardCodedSoilMoistureCount + cntr);
      cntr++;
    }
    
  } // if(soilMoistureModule) 

 // Что мы сделали? Мы добавили N виртуальных датчиков в каждый модуль, основываясь на ранее сохранённой информации.
 // в результате в контроллере появились датчики с показаниями <нет данных>, и показания с них обновятся, как только
 // поступит информация от них с универсальных датчиков.
  
}
void UniRegDispatcher::SaveState()
{
  //Тут сохранение текущего состояния в EEPROM
  uint16_t addr = UNI_SENSOR_INDICIES_EEPROM_ADDR;  
  EEPROM.write(addr++,currentTemperatureCount);
  EEPROM.write(addr++,currentHumidityCount);
  EEPROM.write(addr++,currentLuminosityCount);
  EEPROM.write(addr++,currentSoilMoistureCount);
}

bool UniRegDispatcher::GetRegisteredStates(UniSensorType type, uint8_t sensorIndex, UniSensorState& resultStates)
{
   // смотрим тип сенсора, получаем состояния
   switch(type)
   {
    case uniNone: return false;
    
    case uniTemp: 
    {
        if(!temperatureModule)
          return false; // нет модуля температур в прошивке

       // получаем состояние. Поскольку индексы виртуальных датчиков у нас относительные, то прибавляем
       // к индексу датчика кол-во жёстко прописанных в прошивке. В результате получаем абсолютный индекс датчика в системе.
       resultStates.State1 = temperatureModule->State.GetState(StateTemperature,hardCodedTemperatureCount + sensorIndex);

       return (resultStates.State1 != NULL);
       
    }
    break;
    
    case uniHumidity: 
    {
        if(!humidityModule)
          return false; // нет модуля влажности в прошивке

       resultStates.State1 = humidityModule->State.GetState(StateTemperature,hardCodedHumidityCount + sensorIndex);
       resultStates.State2 = humidityModule->State.GetState(StateHumidity,hardCodedHumidityCount + sensorIndex);

       return (resultStates.State1 != NULL);

    }
    break;
    
    case uniLuminosity: 
    {
        if(!luminosityModule)
          return false; // нет модуля освещенности в прошивке

       resultStates.State1 = luminosityModule->State.GetState(StateLuminosity,hardCodedLuminosityCount + sensorIndex);
       return (resultStates.State1 != NULL);      
    }
    break;
    
    case uniSoilMoisture: 
    {
        if(!soilMoistureModule)
          return false; // нет модуля влажности почвы в прошивке

       resultStates.State1 = soilMoistureModule->State.GetState(StateSoilMoisture,hardCodedSoilMoistureCount + sensorIndex);
       return (resultStates.State1 != NULL);
      
    }
    break;
   } // switch

  return false;    
 
}
bool UniRegDispatcher::RegisterSensor(UniSensorType type, UniSensorState& resultStates, uint8_t& assignedIndex)
{
    assignedIndex = NO_SENSOR_REGISTERED; // говорим, что сенсор не зарегистрирован
  
   // смотрим тип сенсора, назначаем ему индекс, получаем состояния
   switch(type)
   {
    case uniNone: return false;
    
    case uniTemp: 
    {
        // регистрируем температурный датчик
        if(!temperatureModule)
          return false; // нет модуля температур в прошивке

       // добавляем новый датчик температуры в систему
       resultStates.State1 = temperatureModule->State.AddState(StateTemperature,hardCodedTemperatureCount + currentTemperatureCount);

       // передаём датчику его виртуальный индекс
       assignedIndex = currentTemperatureCount;


      // запоминаем индекс, который будет использоваться в дальнейшем 
       currentTemperatureCount++;

       // говорим, что добавили
       return (resultStates.State1 != NULL);
       
    }
    break;
    
    case uniHumidity: 
    {
     // регистрируем датчик влажности
        if(!humidityModule)
          return false; // нет модуля влажности в прошивке

       // добавляем новый датчик температуры в систему
       resultStates.State1 = humidityModule->State.AddState(StateTemperature,hardCodedHumidityCount + currentHumidityCount);
       // добавляем новый датчик влажности в систему
       resultStates.State2 = humidityModule->State.AddState(StateHumidity,hardCodedHumidityCount + currentHumidityCount);

       // передаём датчику его виртуальный индекс
       assignedIndex = currentHumidityCount;

      // запоминаем индекс, который будет использоваться в дальнейшем 
       currentHumidityCount++;

       // говорим, что добавили
       return (resultStates.State1 != NULL);
    }
    break;
    
    case uniLuminosity: 
    {
        // регистрируем датчик освещенности
        if(!luminosityModule)
          return false; // нет модуля освещенности в прошивке

       // добавляем новый датчик освещенности в систему
       resultStates.State1 = luminosityModule->State.AddState(StateLuminosity,hardCodedLuminosityCount + currentLuminosityCount);

       // передаём датчику его виртуальный индекс
       assignedIndex = currentLuminosityCount;

      // запоминаем индекс, который будет использоваться в дальнейшем 
       currentLuminosityCount++;

       // говорим, что добавили
       return (resultStates.State1 != NULL);
      
    }
    break;
    
    case uniSoilMoisture: 
    {
        // регистрируем датчик влажности почвы
        if(!soilMoistureModule)
          return false; // нет модуля влажности почвы в прошивке

       // добавляем новый датчик влажности почвы в систему
       resultStates.State1 = soilMoistureModule->State.AddState(StateSoilMoisture,hardCodedSoilMoistureCount + currentSoilMoistureCount);

       // передаём датчику его виртуальный индекс
       assignedIndex = currentSoilMoistureCount;

      // запоминаем индекс, который будет использоваться в дальнейшем 
       currentSoilMoistureCount++;

       // говорим, что добавили
       return (resultStates.State1 != NULL);
      
    }
    break;
   } // switch

  return false;
}
uint8_t UniRegDispatcher::GetControllerID()
{
  return mainController->GetSettings()->GetControllerID(); 
}


// общий для всех датчиков скратчпад. Эту парадигму можно использовать
// для датчиков, постоянно висящих на линии, потому что опрос всех датчиков
// происходит последовательно. Для класса регистрации датчиков
// должен быть свой скратчпад, поскольку между запросом на чтение
// скратчпада (например, из конфигуратора) и другим действием пользователя
// может пройти некоторое время, в результате чего данные в скратчпаде
// станут невалидными в том случае, если будет использоваться общий
// скратчпад. 
uint8_t UNI_SHARED_SCRATCHPAD[UNI_SCRATCH_SIZE] = {NO_SENSOR_REGISTERED};

AbstractUniSensor::AbstractUniSensor(uint8_t* scratchAddress)
{
  pin = 0;
  rfEnabled = true;
  isScratchpadReaded = false;
  scratchpadAddress = scratchAddress; 

  if(scratchpadAddress && scratchpadAddress != UNI_SHARED_SCRATCHPAD)
    memset(scratchpadAddress,NO_SENSOR_REGISTERED,UNI_SCRATCH_SIZE);
}
void AbstractUniSensor::SetPin(uint8_t pinNumber)
{
  pin = pinNumber;
}

bool AbstractUniSensor::ReadScratchpad() // читаем скратчпад
{
  isScratchpadReaded = false;
  
  if(!pin || !scratchpadAddress)
    return isScratchpadReaded;  
    
    OneWire ow(pin);
    
    if(!ow.reset()) // нет датчика на линии
      return isScratchpadReaded; 
      
    // сначала запускаем конвертацию, чтобы в следующий раз нам вернулись актуальные данные
    ow.write(UNI_START_MEASURE); // посылаем команду на старт измерений
    ow.reset(); // говорим, что команда послана полностью
    
    // теперь читаем скратчпад
    ow.write(UNI_READ_SCRATCHPAD); // посылаем команду на чтение скратчпада
    
    // читаем скратчпад
    for(uint8_t i=0;i<UNI_SCRATCH_SIZE;i++)
      scratchpadAddress[i] = ow.read();
      
    // проверяем контрольную сумму
    isScratchpadReaded = OneWire::crc8(scratchpadAddress, UNI_SCRATCH_SIZE-1) == scratchpadAddress[UNI_SCRATCH_SIZE-1]; 

    return isScratchpadReaded;
}

bool AbstractUniSensor::WriteScratchpad() // пишем в скратчпад
{
  if(!pin || !isScratchpadReaded || !scratchpadAddress)
    return false;
    
   OneWire ow(pin);
   if(!ow.reset()) // нет датчика на линии
    return false;
    
  scratchpadAddress[CONTROLLER_ID_IDX] = UniDispatcher.GetControllerID(); // выставляем ID нашего контроллера
  
  //TODO: Тут включение или выключение радио в зависимости от флага rfEnabled !!!


  //TODO: Подсчёт контрольной суммы ????
  
  ow.write(UNI_WRITE_SCRATCHPAD); // говорим, что хотим записать скратчпад
  
  // теперь пишем данные
   for(uint8_t i=0;i<UNI_SCRATCH_SIZE;i++)
    ow.write(scratchpadAddress[i]);
  
  // говорим, что всё записали
  ow.reset();
  return true;    
}
bool AbstractUniSensor::SetRFState(bool enabled)
{
  if(!isScratchpadReaded)
    return false;  

  rfEnabled = enabled;

  return WriteScratchpad();
}
bool AbstractUniSensor::IsRegistered()
{
  return (isScratchpadReaded && (GetRegistrationID() == UniDispatcher.GetControllerID()));
}

uint8_t AbstractUniSensor::GetRegistrationID()
{
  return scratchpadAddress[CONTROLLER_ID_IDX];
}

uint8_t AbstractUniSensor::GetID()
{
  return scratchpadAddress[RF_ID_IDX];
}
void AbstractUniSensor::ReBindSensor(uint8_t scratchIndex,uint8_t newSensorIndex)
{
  if(!isScratchpadReaded || !scratchpadAddress)
    return;

   // назначаем новый индекс для датчика в системе
   uint8_t idx = DATA_START_IDX + scratchIndex*6; // начало данных
   scratchpadAddress[idx] = newSensorIndex;    
}
bool AbstractUniSensor::GetSensorInfo(uint8_t scratchIndex,uint8_t& sensorType,uint8_t& sensorIndex)
{
   if(!isScratchpadReaded || !scratchpadAddress)
    return false;

   uint8_t idx = DATA_START_IDX + scratchIndex*6; // начало данных
   sensorIndex = scratchpadAddress[idx];
   sensorType = scratchpadAddress[idx+1];

   return true;
   
}
bool AbstractUniSensor::SensorSetup()
{
   if(!isScratchpadReaded || !scratchpadAddress)
    return false;

  uint8_t registeredSensorsCount = 0;
  uint8_t idx = DATA_START_IDX; // начало данных

  for(uint8_t i=0;i<UNI_SENSORS_COUNT;i++, idx += 6)
  {
      UniSensorType sensorType = (UniSensorType) scratchpadAddress[idx+1];
      uint8_t sensorIndex = scratchpadAddress[idx];
      
      if(uniNone != sensorType)
      {
        if(UniDispatcher.GetRegisteredStates(sensorType,sensorIndex, States[i]))
        {
          registeredSensorsCount++;
        }
      } // if
  } // for

  return registeredSensorsCount > 0;
}

void AbstractUniSensor::UpdateData(bool isSensorOnline)
{
    for(uint8_t i=0;i<UNI_SENSORS_COUNT;i++)
    {
      UniSensorState* state = &(States[i]);      
      UpdateStateData(state, i, isSensorOnline);
    } // for
}
void AbstractUniSensor::UpdateStateData(UniSensorState* state,uint8_t idx, bool isSensorOnline)
{
  if(!(state->State1 || state->State2))
    return; // не найдено ни одного состояния  

    uint8_t data_idx = DATA_START_IDX + idx*6; // начало данных для нужного типа сенсора

    UpdateOneState(state->State1,&(scratchpadAddress[data_idx]),isSensorOnline);
    UpdateOneState(state->State2,&(scratchpadAddress[data_idx]),isSensorOnline);
    
}
void AbstractUniSensor::UpdateOneState(OneState* os, uint8_t* data, bool isSensorOnline)
{
    if(!os)
      return;

   uint8_t sensorIndex = *data++;
   uint8_t sensorType = *data++;

   if(sensorIndex == NO_SENSOR_REGISTERED || sensorType == NO_SENSOR_REGISTERED || sensorType == uniNone)
    return; // нет сенсора вообще

   switch(os->GetType())
   {
      case StateTemperature:
      {
        if(sensorType == uniHumidity) // если тип датчика - влажность, значит температура у нас идёт после влажности, в 3-м и 4-м байтах
        {
          data++; data++;
        }
        
        int8_t b1 = isSensorOnline ? (int8_t)*data++ : NO_TEMPERATURE_DATA;
        uint8_t b2 = isSensorOnline ? *data : 0;
        Temperature t(b1, b2);
        os->Update(&t);
        
      }
      break;

      case StateHumidity:
      case StateSoilMoisture:
      {
        int8_t b1 = isSensorOnline ? (int8_t)*data++ : NO_TEMPERATURE_DATA;
        uint8_t b2 = isSensorOnline ? *data : 0;
        Humidity h(b1, b2);
        os->Update(&h);        
      }
      break;

      case StateLuminosity:
      {
        unsigned long lum = NO_LUMINOSITY_DATA;
        if(isSensorOnline)
          memcpy(&lum,data,4);

        os->Update(&lum);
        
      }
      break;

      case StateWaterFlowInstant:
      case StateWaterFlowIncremental:
      #ifdef SAVE_RELAY_STATES
      case StateRelay:
      #endif
      
      break;
      
    
   } // switch
}

bool  AbstractUniSensor::SensorRegister(bool rfTransmitterEnabled)
{
  if(!isScratchpadReaded)
    return false;
  
  uint8_t idx = DATA_START_IDX; // начало данных
  uint8_t assignedIndex = 0;

  uint8_t registeredSensorsCount = 0;

  rfEnabled = rfTransmitterEnabled;
  
  for(uint8_t i=0;i<UNI_SENSORS_COUNT;i++, idx += 6)
  {
      UniSensorType sensorType = (UniSensorType) scratchpadAddress[idx+1];
      if(uniNone != sensorType)
      {
        if(UniDispatcher.RegisterSensor(sensorType,States[i],assignedIndex))
        {
          scratchpadAddress[idx] = assignedIndex;
          registeredSensorsCount++;
        }
      } // if
  } // for 

   // пишем данные в датчик, чтобы он их запомнил
   if(registeredSensorsCount)
   {
      UniDispatcher.SaveState(); // сохраняем состояние
      return WriteScratchpad();
   }

   return false;
}


UniPermanentSensor::UniPermanentSensor(uint8_t pinNumber) : AbstractUniSensor(UNI_SHARED_SCRATCHPAD)
{
  SetPin(pinNumber);
  timer = random(0,UNI_SENSOR_UPDATE_INTERVAL); // разнесём опрос датчиков по времени
  isSensorInited = false;
  canWork = false;
}
void UniPermanentSensor::Update(uint16_t dt)
{
  timer += dt;

  if(timer < UNI_SENSOR_UPDATE_INTERVAL) // рано обновлять
    return;

  timer -= UNI_SENSOR_UPDATE_INTERVAL; // сбрасываем таймер

  // пытаемся читать с датчика
  if(ReadScratchpad()) // датчик есть на линии
  {
    // смотрим, настраивали ли мы датчик до этого или нет?
    if(!isSensorInited)
    {
      isSensorInited = true;
      // не настраивали, настраиваем
      if(IsRegistered())
      {
         // датчик зарегистрирован у нас, просто читаем состояния
         canWork = SensorSetup();
         if(canWork)
          SetRFState(false); // выключаем радиопередатчик
      }
      else
      {
        // датчик не зарегистрирован у нас, регистрируем
        canWork = SensorRegister(false); // регистрируем, попутно выключая радиопередатчик
        
      } // else
      
    } // !isSensorInited

    if(canWork)
    {
      // можем работать, обновляем данные
       UpdateData(true); 
    } // if canWork
    
  } // if(ReadScratchpad())
  else
  {
    // данные прочитать не удалось, возможно, датчик отвалился
    // тупо обновляем данные, говоря, что датчика нет на линии
    UpdateData(false);   

  } // else

  
}

UniRegistrationLine::UniRegistrationLine(uint8_t pinNumber, bool autoreg) : AbstractUniSensor(new uint8_t[UNI_SCRATCH_SIZE])
{
  SetPin(pinNumber);
  lastCheckedSensor = -1;
  timer = 0;
  autoRegistrationMode = autoreg;
  
  memset(rebindedSensorIndicies,NO_SENSOR_REGISTERED,sizeof(rebindedSensorIndicies));
}
void UniRegistrationLine::SetAutoRegistrationMode(bool autoreg)
{
  autoRegistrationMode = autoreg; // запоминаем режим регистрации
  lastCheckedSensor = -1; // сбрасываем ID последнего опрошенного датчика, чтобы при смене режима однозначно вычитать скратчпад у подсоединённого датчика
}
void UniRegistrationLine::Update(uint16_t dt)
{ 

  if(!autoRegistrationMode) // ручной режим регистрации
    return;
  
  timer += dt;
  if(timer < UNI_SENSOR_REGISTER_QUERY_INTERVAL) // не пришло время
    return;

  timer -= UNI_SENSOR_REGISTER_QUERY_INTERVAL; // сбрасываем таймер
  
 // пытаемся читать с линии
  if(ReadScratchpad()) // датчик есть на линии
  {
    // смотрим, настраивали ли мы датчик до этого или нет?
    if(lastCheckedSensor != GetID()) // новый датчик, который мы ещё не проверяли
    {
      lastCheckedSensor = GetID(); // сохраняем, чтобы не проверять повторно
      
      //  проверяем, надо ли настраивать
      if(IsRegistered())
      {
         // датчик зарегистрирован у нас, просто включаем ему радиопередатчик
          SetRFState(true);
      }
      else
      {
        // датчик не зарегистрирован у нас, регистрируем
        SensorRegister(true); // регистрируем, попутно включая ему радиопередатчик
        
      } // else
      
    } //  if(lastCheckedSensor != GetID())
    
  } // if(ReadScratchpad())
  else
  {
    // данные прочитать не удалось, никакого датчика нет на линии
   lastCheckedSensor = -1; // сбрасываем индекс последнего проверенного сенсора

  } // else
  
}
void UniRegistrationLine::SetSensorIndex(uint8_t scrathIndex, uint8_t sensorIndex)
{
  if(scrathIndex < UNI_SENSORS_COUNT)
    rebindedSensorIndicies[scrathIndex] = sensorIndex;
}
void UniRegistrationLine::SaveConfiguration()
{
  bool anyRebinded = false;
  for(uint8_t i=0;i<UNI_SENSORS_COUNT;i++)
  {
    if(rebindedSensorIndicies[i] != NO_SENSOR_REGISTERED)
    {
      anyRebinded = true;
      ReBindSensor(i, rebindedSensorIndicies[i]);
    }
      
  } // for

  if(anyRebinded)
    WriteScratchpad();
}
bool UniRegistrationLine::IsSensorPresent()
{
  // читаем скратчпад, и если прочитали - значит датчик на линии
  // при этом состояние скратчпада останется валидным, т.к.
  // мы используем свой скратчпад, отличный от скратчпада
  // для сенсоров, постоянно висящих на линии (они юзают общий для всех).
  return ReadScratchpad(); 
}

