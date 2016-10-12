module.exports = [
  {
    "type": "heading",
    "defaultValue": "Step in Time Settings"
  },
  {
    "type": "text",
    "defaultValue": "I will be adding more settings in the future."
  },
    {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Watchface Settings"
      },
      {
        "type": "toggle",
        "messageKey": "STEPGOALENABLED",
        "label": "Buzz when you achieve the below Step Goals?",
        "defaultValue": true
      },
      {
        "type": "slider",
        "messageKey": "STEPMIDGOAL",
        "defaultValue": 4500,
        "label": "Intermediate Step Goal",
        "description": "Set your intermediate step goal and your watch will buzz when you achieve that goal.",
        "min": 1000,
        "max": 15000,
        "step": 500
      },
      {
        "type": "slider",
        "messageKey": "STEPGOAL",
        "defaultValue": 7500,
        "label": "Daily Step Goal",
        "description": "Set your daily step goal and your watch will buzz when you achieve that goal.",
        "min": 5000,
        "max": 20000,
        "step": 500
      },
      {
        "type": "toggle",
        "messageKey": "GOALTYPE",
        "label": "When set to True the circle will fill as you achieve your average daily steps, when set to False the circle will fill according to your daily step goal specified above",
        "defaultValue": true
      },
      {
        "type": "slider",
        "messageKey": "MANUALSLEEPSTART",
        "defaultValue": 24,
        "label": "Manually set the watchface to go to sleep",
        "description": "This setting is in military time and will stop the watchface refreshing for 6 hours to save power, flick your wrist to bring the watchface out of sleep mode.  Set to 24 to disable.",
        "min": 0,
        "max": 24,
        "step": 1
      },
      {
      "type": "color",
      "messageKey": "TEXTCOLOR",
      "defaultValue": "FFAA00",
      "label": "Press the color dot to the right to select a color used to display text and the weather icon",
      "sunlight": true,
      }
    ]
    },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Watchface Weather Settings"
      },
      {
        "type": "radiogroup",
        "messageKey": "WeatherProvide",
        "label": "Please select which weather provider you would like to use",
        "defaultValue": "",
        "options": [
          { 
          "label": "Do not display Weather", 
          "value": ""
          },
          { 
          "label": "OpenWeatherMap", 
          "value": "OpenWe" 
          },
          { 
          "label": "WeatherUnderGround", 
          "value": "WUnder" 
          },
          { 
          "label": "Forecast.io", 
          "value": "For.io" 
          }]
      },
      {
        "type": "input",
        "messageKey": "APIKEY",
        "defaultValue": "",
        "label": "Please sign up for an API Key from your selected weather provider and enter it here"
      },
      {
        "type": "toggle",
        "messageKey": "FTICK",
        "label": "On for F, off for C",
        "defaultValue": true
      },

    ]
  },
  {
    "type": "submit",
    "defaultValue": "Save Settings"
  }
];