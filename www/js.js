Zepto(function($) {
    $.getJSON("/channels.json", (data) => {
        if (data.software) {
            document.title = data.software.name + ' v.' + data.software.version;
            $(`<div id="logo" class="noselect animate__animated animate__slideInDown">${data.software.name}</div>`).append(`<div class="version">v.${data.software.version}</div>`).appendTo('#left');
            if (data.channels) {
                $('<ul id="channels" class="noselect animate__animated animate__slideInLeft"></ul>').appendTo('#left');
                data.channels.map((i) => {
                    $(`<li class="${i.status}">${i.name}<div class="strategy">${i.strategy}</div></li>`).appendTo('#channels');
                })
            }
            $log = $('<ul id="log"></ul>');
            $log.appendTo('#right');
            $.getJSON('/log.json', (data) => {
                data.lines.map((i) => {
                    let ts = new Date(i.ts);
                    $(`<li class="l_${i.level}"><b class="ts">${ts.toLocaleTimeString()}</b><span class="message">${i.message}</span></li>`).appendTo($log);
                })
            })
        }
    });
});