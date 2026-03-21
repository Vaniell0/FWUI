# frozen_string_literal: true

module FWUI
  HOT_RELOAD_JS = <<~'JS'
    (function() {
      if (typeof WebSocket === 'undefined') return;
      var proto = location.protocol === 'https:' ? 'wss:' : 'ws:';
      var ws = new WebSocket(proto + '//' + location.host + '/__fwui_ws');
      var reconnectDelay = 1000;

      ws.onmessage = function(event) {
        var msg;
        try { msg = JSON.parse(event.data); } catch(e) { return; }

        if (msg.type === 'reload') {
          console.log('[FWUI] Hot reload:', msg.file);
          location.reload();
        } else if (msg.type === 'css_reload') {
          document.querySelectorAll('link[rel="stylesheet"]').forEach(function(link) {
            var href = link.href;
            link.href = href.split('?')[0] + '?_t=' + Date.now();
          });
          console.log('[FWUI] CSS reloaded');
        }
      };

      ws.onclose = function() {
        console.log('[FWUI] Disconnected. Reconnecting in', reconnectDelay, 'ms');
        setTimeout(function() {
          reconnectDelay = Math.min(reconnectDelay * 2, 10000);
          location.reload();
        }, reconnectDelay);
      };

      ws.onerror = function() { ws.close(); };
    })();
  JS
end
