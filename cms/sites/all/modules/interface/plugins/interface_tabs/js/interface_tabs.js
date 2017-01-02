// this script should contain logic for switching tabs, making them active, and handling the public user interface
(function($) {
Drupal.behaviors.interface_tabs = {
  attach: function (context, settings) {
  // find the default, active tab and make it's contents visible
  var $make_visible = $('.interface_tabs').children().children('SPAN');
  $make_visible.each(function(){
    if($(this).hasClass('active')){
      var $region = $(this).attr('name');
      $('.' + $region).show();
    }
  });
  // change tab display
  $('.interface_tabs').children().not('.add_new').once('switch_tabs', jQuery.interface_tabs_tab_switcher);
  }
};
jQuery.interface_tabs_tab_switcher = function(){
  // make each tab switch when a user clicks
  $(this).click(function(){
    $('.interface_tabs').children().children().removeClass('active');
    $(this).children().addClass('active');
    $('.interface_tab_content_holder').find('.interface_tab_content').hide();
    var $content_name = $(this).children().attr('name');
    $('.interface_tab_content_holder').find('.' + $content_name).show();
  });
};
})(jQuery);
