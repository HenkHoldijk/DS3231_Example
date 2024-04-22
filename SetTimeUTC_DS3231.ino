// standard includes
#include <Wire.h>
#include <Time.h>

// generic library includes
#include <Timezone.h>

// platform (HW) specific includes
#include <DS3231RTC.h>

#define START_DELAY ( 9 )                                   // compilation, upload and booting delay (s)

// month names as reported by the "Arduino" compiler
const char *monthName[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

// call constructors for the Time Zone handling. Central European Time (Frankfurt, Paris)
TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120};     //Central European Summer Time (-2 hours)
TimeChangeRule CET  = {"CET ", Last, Sun, Oct, 3, 60};      //Central European Standard Time (-1 hour)
Timezone CE(CEST, CET);

// call the constructor for the DS3231 RTC module
DS3231RTC rtc = DS3231RTC();

// function prototypes
time_t GetLocalCompileTime();



// ######################################
// # set the DS3231 RTC to run UTC time #
// ######################################
void setup()
{
  tmElements_t tmTimeNow;
  time_t       tLocal;
  time_t       tUtc;
  char         cTmp[20];

  // init the RTC
  rtc.begin();

  // init serial port @ 9600 baud
  Serial.begin(9600);

  // get the local compilation time (extrapolated for now)
  tLocal = GetLocalCompileTime();

  // convert local time to UTC (I live in the CE time zone)
  tUtc = CE.toUTC(tLocal);

  // set the ARDUINO internal time to UTC
  setTime(tUtc);

  // set the RTC (HW) time to UTC
  rtc.setTime(tUtc);

  // report the time (UTC) set
  if (rtc.readTime(tmTimeNow))
  {
    // Show Date & Time
    Serial.print(F("DS3231 configured Date & Time (UTC) : "));
    sprintf(cTmp, "%02d-%02d-%04d", tmTimeNow.Day, tmTimeNow.Month, tmTimeNow.Year+1970);
    Serial.print(cTmp);
    Serial.print(F(" @ "));
    sprintf(cTmp, "%02d:%02d:%02d", tmTimeNow.Hour, tmTimeNow.Minute, tmTimeNow.Second);
    Serial.println(cTmp);
    Serial.println("");
  }
  else
  {
    Serial.println(F("Error: unable to read the TRC time !!!"));
  }
}



// ##########################################
// # report (serial) the current (UTC) time #
// ##########################################
void loop()
{
  tmElements_t tmTimeNow;
  char         cTmp[20];

  if (rtc.readTime(tmTimeNow))
  {
    sprintf(cTmp, "%02d:%02d:%02d (UTC)", tmTimeNow.Hour, tmTimeNow.Minute, tmTimeNow.Second);
    Serial.println(cTmp);
  }
  else
  {
    Serial.println(F("Error: unable to read the TRC time !!!"));
  }

  delay(1000);
}



// ##########################################
// # extract the compile time in local time #
// ##########################################
time_t GetLocalCompileTime()
{
  char         cMonth[12];
  char         cTmp[20];
  int          iDay;
  int          iMonth;
  int          iYear;
  int          iHour;
  int          iMin;
  int          iSec;
  tmElements_t tm;
  time_t       t;
  int          i;

  // extract the compilation date and time
  if (sscanf(__DATE__, "%s %d %d", cMonth, &iDay, &iYear) != 3) return false;
  if (sscanf(__TIME__, "%d:%d:%d", &iHour, &iMin, &iSec) != 3) return false;
  for (i=0; i<12; i++)
  {
    if (strcmp(cMonth, monthName[i]) == 0) break;
  }
  if (i >= 12) return false;
  iMonth = i + 1;

  // construct the tmElements_t time (struct)
  tm.Day    = iDay;
  tm.Month  = iMonth;
  tm.Year   = CalendarYrToTm(iYear);
  tm.Hour   = iHour;
  tm.Minute = iMin;
  tm.Second = iSec;

  // Show compilation Date & Time
  Serial.print(F("Compilation Date & Time (Local) : "));
  sprintf(cTmp, "%02d-%02d-%04d", tm.Day, tm.Month, tm.Year+1970);
  Serial.print(cTmp);
  Serial.print(F(" @ "));
  sprintf(cTmp, "%02d:%02d:%02d", tm.Hour, tm.Minute, tm.Second);
  Serial.println(cTmp);
  Serial.println("");

  // convert tmElements_t time (struct) into time_t time (seconds)
  t = makeTime(tm);

  // compensate for compilation, upload and booting duration
  t += START_DELAY;

  return t;
}
