function Site(salt, password) {
    if (salt && password) {
        var keyIvWA = CryptoJS.EvpKDF(
            password,
            CryptoJS.enc.Utf8.parse(salt), {
                keySize: (32 + 16) / 4,
                iterations: 100,
                hasher: CryptoJS.algo.SHA256
            }
        );
        keyIvWA.clamp();
        this.key = CryptoJS.lib.WordArray.create(keyIvWA.words.slice(0, 32 / 4));
        this.iv = CryptoJS.lib.WordArray.create(keyIvWA.words.slice(32 / 4));
        this.decrypt = function(mess) {
            try {
                return CryptoJS.AES.decrypt({ ciphertext: CryptoJS.enc.Base64.parse(mess) },
                    this.key, { iv: this.iv }
                ).toString(CryptoJS.enc.Utf8);
            } catch (error) {
                console.error(error);
                return "???";
            }
        };
        this.encrypt = function(mess) {
            try {
                return CryptoJS.AES.encrypt(mess,
                    this.key, { iv: this.iv }
                ).toString();
            } catch (error) {
                console.error(error);
                return "";
            }
        };
    }
    this.api = function(url, data, success) {
        if (!$.isFunction(success)) {
            success = data;
        }
        return $.ajax({
            dataType: "json",
            url: url,
            data: data,
            success: success,
            headers: {
                "X-Key": this.encrypt(password)
            }
        });
    }
}


Zepto(function($) {
    window.site = new Site("savostin", "password");
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
            site.api('/log.json', (data) => {
                console.log(site.decrypt(data.funds));
                data.lines.map((i) => {
                    let ts = new Date(i.ts * 1000);
                    $(`<li class="l_${i.level}"><b class="ts">${ts.toLocaleTimeString()}</b><span class="message">${window.site.decrypt(i.message)}</span></li>`).prependTo($log);
                })
            })

        }
    });
});