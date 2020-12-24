"use strict";

Zepto(function ($) {
  $.getJSON("/channels.json", function (data) {
    if (data.software) {
      document.title = data.software.name + ' v.' + data.software.version;
      $("<div id=\"logo\" class=\"noselect animate__animated animate__slideInDown\">".concat(data.software.name, "</div>")).append("<div class=\"version\">v.".concat(data.software.version, "</div>")).appendTo('#left');

      if (data.channels) {
        $('<ul id="channels" class="noselect animate__animated animate__slideInLeft"></ul>').appendTo('#left');
        data.channels.map(function (i) {
          $("<li class=\"".concat(i.status, "\">").concat(i.name, "<div class=\"strategy\">").concat(i.strategy, "</div></li>")).appendTo('#channels');
        });
      }

      $log = $('<ul id="log"></ul>');
      $log.appendTo('#right');
      $.getJSON('/log.json', function (data) {
        data.lines.map(function (i) {
          var ts = new Date(i.ts);
          $("<li class=\"l_".concat(i.level, "\"><b class=\"ts\">").concat(ts.toLocaleTimeString(), "</b><span class=\"message\">").concat(i.message, "</span></li>")).appendTo($log);
        });
      });
    }
  });
});