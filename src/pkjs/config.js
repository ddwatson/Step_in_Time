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
        "label": "Enable Step Goals?",
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
      }
    ]
    },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Watchface Weather Settings Coming Soon"
      }
    ]
  },
  {
    "type": "submit",
    "defaultValue": "Save Settings"
  }
];