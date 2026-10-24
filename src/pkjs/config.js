module.exports = [
  { type: 'heading', defaultValue: 'PebbleBeat' },
  {
    type: 'section',
    items: [
      { type: 'heading', defaultValue: 'Connection' },
      {
        type: 'input',
        messageKey: 'WebsocketServer',
        label: 'WebSocket URL',
        description: 'Use ws:// or wss://, including the port and path if needed.',
        attributes: { type: 'url', required: true, maxlength: 1024, pattern: 'wss?://[^\\s#]+' }
      }
    ]
  },
  { type: 'submit', defaultValue: 'Save Settings' }
];
