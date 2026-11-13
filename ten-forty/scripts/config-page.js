#!/usr/bin/env node
/* Renders the settings page that the phone opens, to docs/config.html.
 *
 *   node scripts/config-page.js --write   # regenerate after changing a field
 *   node scripts/config-page.js --check   # fail if docs/config.html is stale
 *
 * The output is committed because GitHub Pages serves it as a plain file:
 * https://damosullivan.github.io/ten-forty/config.html, which is the URL in
 * CONFIG_URL. iOS refuses to load a data: URI in the config webview, so this
 * file is not optional — it is how iPhone users reach their settings.
 *
 * Nothing here reaches the watch bundle. The phone side (src/pkjs/index.js)
 * only opens this URL and appends the current settings as a fragment.
 *
 * Two things the page cannot know at build time:
 *
 *   - the settings themselves, which arrive in the fragment as JSON;
 *   - the watch's platform. Round displays only offer the two narrow date
 *     formats, so the phone passes _maxDate in the fragment and the page
 *     filters its own <option> list on load. This is why the page renders all
 *     four formats and hides one at runtime rather than baking in a number.
 */
'use strict';

var fs = require('fs');
var path = require('path');

var OUT = path.join(__dirname, '..', '..', 'docs', 'config.html');

/* Mirrors DEFAULT_OPTS in src/pkjs/index.js. The watch always sends all nine
 * keys, so this only matters for the page opened directly by hand. If you add
 * a setting, change both. */
var DEFAULT_OPTS = { Theme: 0, Align: 0, DateAlign: 1, DateFormat: 0, DatePos: 2, OhWords: 1, Weight: 0, Case: 0, Transition: 2 };

var KEYS = ["Theme", "Align", "DateAlign", "DateFormat", "DatePos", "OhWords", "Weight", "Case", "Transition"];

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

  /* All four formats are rendered; the round ones are removed on load. The
   * circle leaves the date an 8-character chord at the top, and the text layer
   * truncates with an ellipsis rather than wrapping, so offering them on a
   * round watch would only produce a broken layout. The watch clamps the same
   * values, so a setting carried over from a rectangular watch cannot break it. */
  var dateFormats = [
    ['0', 'Tue 10'],
    ['1', '10 Nov'],
    ['2', 'Tue 10 Nov'],
    ['3', 'Tuesday']
  ];

  var fields = [
    ['Theme', 'Theme', [['0', 'White on black'], ['1', 'Black on white']]],
    ['Align', 'Time alignment', [['0', 'Left'], ['1', 'Centre'], ['2', 'Right']]],
    ['DateAlign', 'Date alignment', [['0', 'Left'], ['1', 'Centre'], ['2', 'Right']]],
    ['DateFormat', 'Date format', dateFormats],
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
    'var K=' + JSON.stringify(KEYS) + ';',
    /* round displays get the two narrow formats only */
    'var m=(o._maxDate===undefined)?3:o._maxDate;',
    'var df=document.getElementById("DateFormat");',
    'for(var i=df.options.length-1;i>=0;i--){',
    '  if(parseInt(df.options[i].value,10)>m)df.removeChild(df.options[i]);',
    '}',
    'K.forEach(function(k){',
    '  var e=document.getElementById(k);',
    '  e.value=String(o[k]===undefined?D[k]:o[k]);',
    /* a format this watch does not offer, carried over from another one */
    '  if(!e.value)e.value=String(D[k]);',
    '});',
    'document.getElementById("save").addEventListener("click",function(){',
    '  var out={};',
    '  K.forEach(function(k){',
    '    out[k]=parseInt(document.getElementById(k).value,10);',
    '  });',
    '  document.location="pebblejs://close#"+encodeURIComponent(JSON.stringify(out));',
    '});'
  ].join('');

  return '<!doctype html><html><head><meta charset="utf-8">' +
         '<meta name="viewport" content="width=device-width,initial-scale=1">' +
         '<title>Ten Forty</title>' +
         '<style>' + css + '</style></head><body>' +
         '<h1>Ten Forty</h1>' + body +
         '<button id="save">Save</button>' +
         '<script>' + js + '<\/script></body></html>\n';
}

var html = buildConfigHtml();
var mode = process.argv[2];

if (mode === '--write') {
  fs.mkdirSync(path.dirname(OUT), { recursive: true });
  fs.writeFileSync(OUT, html);
  console.log('wrote ' + path.relative(process.cwd(), OUT) + ' (' + html.length + ' bytes)');
} else if (mode === '--check') {
  var current = null;
  try { current = fs.readFileSync(OUT, 'utf8'); } catch (e) { /* missing counts as stale */ }
  if (current !== html) {
    console.error('docs/config.html is out of date with scripts/config-page.js');
    console.error('run: node scripts/config-page.js --write');
    process.exit(1);
  }
  console.log('config page is up to date');
} else {
  console.error('usage: node scripts/config-page.js --write|--check');
  process.exit(2);
}
