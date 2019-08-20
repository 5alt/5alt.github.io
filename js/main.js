(function($){
    var toTop = ($('#sidebar').height() - $(window).height()) + 60;
    // Caption
    $('.article-entry').each(function(i) {
        $(this).find('img').each(function() {
            if (this.alt && !(!!$.prototype.justifiedGallery && $(this).parent('.justified-gallery').length)) {
                $(this).after('<span class="caption">' + this.alt + '</span>');
            }

            // 对于已经包含在链接内的图片不适用lightGallery
            if ($(this).parent().prop("tagName") !== 'A') {
                $(this).wrap('<a href="' + this.src + '" title="' + this.alt + '" class="gallery-item"></a>');
            }
        });
    });
    if (lightGallery) {
        var options = {
            selector: '.gallery-item',
        };
        $('.article-entry').each(function(i, entry) {
            lightGallery(entry, options);
        });
        lightGallery($('.article-gallery')[0], options);
    }
    if (!!$.prototype.justifiedGallery) {  // if justifiedGallery method is defined
        var options = {
            rowHeight: 140,
            margins: 4,
            lastRow: 'justify'
        };
        $('.justified-gallery').justifiedGallery(options);
    }

    // Profile card
    $(document).on('click', function () {
        $('#profile').removeClass('card');
    }).on('click', '#profile-anchor', function (e) {
        e.stopPropagation();
        $('#profile').toggleClass('card');
    }).on('click', '.profile-inner', function (e) {
        e.stopPropagation();
    });

    // To Top
    if ($('#sidebar').length) {
        $(document).on('scroll', function () {
            if ($(document).width() >= 800) {
                if(($(this).scrollTop() > toTop) && ($(this).scrollTop() > 0)) {
                    $('#toTop').fadeIn();
                    $('#toTop').css('left', $('#sidebar').offset().left);
                } else {
                    $('#toTop').fadeOut();
                }
            } else {
                $('#toTop').fadeIn();
                $('#toTop').css('right', 20);
            }
        }).on('click', '#toTop', function () {
            $('body, html').animate({ scrollTop: 0 }, 600);
        });
    }

})(jQuery);


(function(){
    function set_image_size(image, width, height) 
{
    image.setAttribute("width", width + "px");
    image.setAttribute("height", height + "px");
}

function hexo_resize_image()
{
    var imgs = document.getElementsByTagName('img');
    for (var i = imgs.length - 1; i >= 0; i--) 
    {
        var img = imgs[i];

        var src = img.getAttribute('src').toString();

        var fields = src.match(/(?<=\?)\d*x\d*/g);
        if (fields && fields.length == 1)
        {
            var values = fields[0].split("x");
            if (values.length == 2)
            {
                var width = values[0];
                var height = values[1];

                if (!(width.length && height.length))
                {
                    var n_width = img.naturalWidth;
                    var n_height = img.naturalHeight;
                    if (width.length > 0)
                    {
                        height = n_height*width/n_width;
                    }
                    if (height.length > 0)
                    {
                        width = n_width*height/n_height;
                    }
                }
                set_image_size(img, width, height);
            }
            continue;
        }

        fields = src.match(/(?<=\?)\d*/g);
        if (fields && fields.length == 1)
        {
            var scale = parseFloat(fields[0].toString());
            var width = scale/100.0*img.naturalWidth;
            var height = scale/100.0*img.naturalHeight;
            set_image_size(img, width, height);
        }
    }
}
hexo_resize_image()
})()
