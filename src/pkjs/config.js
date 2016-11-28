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
      "type": "color",
      "messageKey": "TEXTCOLOR",
      "defaultValue": "FFAA00",
      "label": "Press the color dot to the right to select a color used to display text and the weather icon",
      "sunlight": true,
      }
    ]
    },{
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Power Saving Settings"
      },
      {
        "type": "toggle",
        "messageKey": "SLEEPENABLED",
        "label": "When enabled, the watch detects when you are sleeping and will stop the display showing 'Watch is sleeping' and set the time to 8888 to save power.  Flick your wrist to immediately wake the watckface",
        "defaultValue": true
      },
      {
        "type": "toggle",
        "messageKey": "HIBERNATEENABLED",
        "label": "When enabled, the watch detects when there is no significant movement for 30 minutes (you couch potato) and will stop the display showing 'Watch is idle' and set the time to 8888 to save power.  Flick your wrist to immediately wake the watckface",
        "defaultValue": true
      }
    ]
      },{
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