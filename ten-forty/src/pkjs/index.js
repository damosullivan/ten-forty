/* Ten Forty - settings plumbing
 *
 * The config page is embedded below and handed to the phone as a data: URI,
 * which works on Android and in the emulator. For iOS (and for a published
 * listing) host config.html somewhere and swap CONFIG_URL for its https URL.
 */

var CONFIG_URL = null; /* set to 'https://example.com/config.html' if hosted */

var DEFAULT_OPTS = { Theme: 0, Align: 0, DateAlign: 1, DatePos: 2, OhWords: 1, Weight: 0, Case: 0, Transition: 2 };

function loadOpts() {
  try {
    var raw = localStorage.getItem('ten-forty');
    if (raw) {
      var o = JSON.parse(raw);
      var out = {};
      for (var k in DEFAULT_OPTS) {
        out[k] = (o[k] === undefined) ? DEFAULT_OPTS[k] : o[k];
      }
      return out;
    }
  } catch (e) { /* ignore */ }
  return JSON.parse(JSON.stringify(DEFAULT_OPTS));
}

function saveOpts(o) {
  try { localStorage.setItem('ten-forty', JSON.stringify(o)); } catch (e) {}
}

function sendOpts(o) {
  Pebble.sendAppMessage(o, function () {
    console.log('settings sent');
  }, function (e) {
    console.log('settings failed: ' + JSON.stringify(e));
  });
}

function esc(s) {
  return String(s).replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;');
}

function buildConfigHtml() {
  var css = [
    'body{background:#111;color:#eee;font:15px/1.4 -apple-system,Roboto,sans-serif;margin:0;padding:18px}',
    'h1{font-size:17px;font-weight:600;margin:0 0 18px}',
    'label{display:block;font-size:12px;letter-spacing:.08em;text-transform:uppercase;color:#8a8a92;margin:16px 0 6px}',
    'select{width:100%;padding:10px;font-size:15px;background:#1e1e22;color:#eee;border:1px solid #333;border-radius:6px}',
    'button{width:100%;margin-top:26px;padding:14px;font-size:15px;font-weight:600;background:#eee;color:#111;border:0;border-radius:6px}'
  ].join('');

  var fields = [
    ['Theme', 'Theme', [['0', 'White on black'], ['1', 'Black on white']]],
    ['Align', 'Time alignment', [['0', 'Left'], ['1', 'Centre'], ['2', 'Right']]],
    ['DateAlign', 'Date alignment', [['0', 'Left'], ['1', 'Centre'], ['2', 'Right']]],
    ['DatePos', 'Date', [['0', 'Hidden'], ['1', 'Top'], ['2', 'Bottom']]],
    ['OhWords', 'Single-digit minutes', [['1', 'Seven oh five'], ['0', 'Seven five']]],
    ['Weight', 'Font weight', [['0', 'Light'], ['1', 'Regular']]],
    ['Case', 'Letter case', [['0', 'lowercase'], ['1', 'UPPERCASE'], ['2', 'Capital']]],
    ['Transition', 'Transition', [['2', 'Slide the hour only'], ['1', 'Slide every row'], ['0', 'No animation']]]
  ];

  var body = '';
  for (var i = 0; i < fields.length; i++) {
    var f = fields[i];
    body += '<label>' + esc(f[1]) + '</label><select id="' + f[0] + '">';
    for (var j = 0; j < f[2].length; j++) {
      body += '<option value="' + f[2][j][0] + '">' + esc(f[2][j][1]) + '</option>';
    }
    body += '</select>';
  }

  var js = [
    'var o=JSON.parse(decodeURIComponent((location.hash||location.search).slice(1))||"{}");',
    'var D=' + JSON.stringify(DEFAULT_OPTS) + ';',
    '["Theme","Align","DateAlign","DatePos","OhWords","Weight","Case","Transition"].forEach(function(k){',
    '  var e=document.getElementById(k);',
    '  e.value=String(o[k]===undefined?D[k]:o[k]);',
    '});',
    'document.getElementById("save").addEventListener("click",function(){',
    '  var out={};',
    '  ["Theme","Align","DateAlign","DatePos","OhWords","Weight","Case","Transition"].forEach(function(k){',
    '    out[k]=parseInt(document.getElementById(k).value,10);',
    '  });',
    '  document.location="pebblejs://close#"+encodeURIComponent(JSON.stringify(out));',
    '});'
  ].join('');

  return '<!doctype html><html><head><meta charset="utf-8">' +
         '<meta name="viewport" content="width=device-width,initial-scale=1">' +
         '<style>' + css + '</style></head><body>' +
         '<h1>Ten Forty</h1>' + body +
         '<button id="save">Save</button>' +
         '<script>' + js + '<\/script></body></html>';
}

/* The watch is the source of truth. It pushes its settings on launch and we
 * mirror them here, so the config page always opens on the real values. The
 * phone must not push settings on startup: that used to overwrite whatever the
 * watch had, which made the face appear to change on its own. */
Pebble.addEventListener('appmessage', function (e) {
  var payload = e.payload || {};
  var opts = loadOpts();
  var changed = false;
  for (var k in opts) {
    if (payload[k] !== undefined && payload[k] !== opts[k]) {
      opts[k] = payload[k];
      changed = true;
    }
  }
  if (changed) saveOpts(opts);
});

Pebble.addEventListener('showConfiguration', function () {
  var opts = loadOpts();
  var url;
  if (CONFIG_URL) {
    url = CONFIG_URL + '#' + encodeURIComponent(JSON.stringify(opts));
  } else {
    /* the '#' must stay outside the encoded payload, otherwise the settings
     * JSON becomes part of the document instead of the fragment */
    url = 'data:text/html;charset=utf-8,' +
          encodeURIComponent(buildConfigHtml()) +
          '#' + encodeURIComponent(JSON.stringify(opts));
  }
  Pebble.openURL(url);
});

Pebble.addEventListener('webviewclosed', function (e) {
  if (!e.response) return;
  var opts;
  try { opts = JSON.parse(decodeURIComponent(e.response)); } catch (err) { return; }
  saveOpts(opts);
  sendOpts(opts);
});
