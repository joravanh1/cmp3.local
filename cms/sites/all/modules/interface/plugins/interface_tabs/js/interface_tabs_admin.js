// this script should contain logic for switching tabs, making them active, and handling the public user interface
(function ($) {

    // "use strict";

Drupal.behaviors.interface_tabs_admin = {
  attach: function (context, settings) {
	
    // make the list of tabs sortable
	$('.interface_tabs').once('reorder_tabs', function(){
	  $(this).sortable({
      stop: interface_reordered_tabs
	  });
	});
	
	// initialize all the tabs
	$('.interface_tabs').children().not('.add_new').once(interface_tabs_admin_tab_init);
	
	// add behavior for creating a new tab
	$('.interface_tabs > .add_new').once(interface_tabs_admin_create_new_tab);
	
	// make all regions sortable
	interface_tabs_admin_sortable_regions();
  }

};

// initializes a tab
// adds renaming, swtiching and dropping behaviors
function interface_tabs_admin_tab_init(){
  // swtich between tabs
  $(this).once('switch_tabs', jQuery.interface_tabs_tab_switcher);
  // change tab name
  // keep this off Hidden Fields
  if($(this).find('SPAN').text() != 'Hidden Fields'){
    $(this).once('change_tab_name', interface_tabs_admin_rename_tab);
  }
  // make it droppable
  $(this).once('droppable_tab', interface_tabs_admin_droppable_tab);
  // make it unselectable
  // $(this).disableSelection();
}

//makes tab regions sortable. needs to be called when the page loads and each time a new tab is added
function interface_tabs_admin_sortable_regions(){
  $('.interface_tab_region').once('sort_behavior', function () {
	$(this).sortable({
	  placeholder: "interface_placeholder",
	  opacity: ".9",
          stop: interface_reordered_tabs
	});
	$(this).disableSelection();
  });
}	

// creates a new tab, initializes it, and creates regions for storing form elements
function interface_tabs_admin_create_new_tab (){
  $(this).click(function(){
	var $tabcount = $('.interface_tabs').children().length-1;
	var $insert = '<li class="interface_tab_' + $tabcount + ' drop_behavior-processed ui-droppable"><span class="interface_tab" name="content_tab_' + $tabcount + '">New Tab</span></li>';
	$(this).before($insert);
		
	// add standard behaviors to new tab
	var $new_item = $('.interface_tab_' + $tabcount);
	// TODO: create the content region for the new tab
	$('.interface_tab_content_holder').append('<div class="interface_tab_content content_tab_' + $tabcount + '"><div class="interface_tab_region" style=""></div></div>');
	$new_item.once(interface_tabs_admin_tab_init);
	
	// make all tab regions sortable
	// do this here in order to 
	  interface_tabs_admin_sortable_regions();
	
  });
}

// makes a tab droppable
function interface_tabs_admin_droppable_tab(){
  $(this).droppable({
    drop: interface_tabs_admin_drop_item,
    tolerance: 'pointer',
    hoverClass: 'interface_hover',
    accept: '.interface_instance',
  });
};


// triggered when dropping an element on a tab
function interface_tabs_admin_drop_item(event, ui ){
	
  // get the name of the tab region to place the element then create the jquery object for placing
  var $placer = $(this).find('span').attr('name');
  var $check = $('.' + $placer).find('.interface_tab_region');
  
  // ui.draggable is what we are trying to move
  var $item = $(ui.draggable);
  
  // place the element there. create a clone, there is an issue with directly placing the element
  $check.prepend($item.clone().attr('style', ''));
  
  // remove the original element
  $item.remove();
}

// TODO: add function for renaming a tab
function interface_tabs_admin_rename_tab(){
  
  $(this).dblclick(function(){
    
    // set the name to the value of the current tab
    // only allow this to be set once at a time
    if($('#interface-tab-edit-form').length == 0){
      
      // hide the contents of the field 
      $(this).find('span').hide().end().append(Drupal.settings.interfaceTabEditForm);
      
      $('#interface-tab-edit-form').find('#edit-edit-title').attr('value', $(this).find('span').text());
      
      // TODO: set the tab_id for the current tab
      // right now, this is just using the basic name
      // where do we store the tab_id? need a unique identifier
      $('#interface-tab-edit-form').find('input[name="tab_id"]').attr('value', $(this).find('span').text());
      
      // make the form submit
     $('#interface-tab-edit-form').submit(interface_tabs_admin_tab_edit_form_submit);

     Drupal.attachBehaviors($('#interface-tab-edit-form'));
      
    }
    
  });
}

function interface_tabs_admin_tab_edit_form_submit(){
  // set the value of the span and display it
  // $('#interface-tab-edit-form').find('#edit-edit-title').attr('value', $(this).find('span').text());
  $('#interface-tab-edit-form').parent().find('SPAN').text($('#interface-tab-edit-form').find('#edit-edit-title').attr('value'));
  
  // get a list of all the tabs, in order, and the titles associated with them. This will be saved to the database
  var $tabs = interface_tabs_admin_get_tab_order();
  var $regions = interface_tabs_admin_get_regions();
  var $data = JSON.stringify({ tabs: $tabs, regions: $regions });
  
  // set all the data about tabs and regions to a hidden field
  $('#interface-tab-edit-form').find('input[name="interface_data"]').attr('value', $data);
  
  // submit the form and remove it
  interface_tabs_admin_save_tab_order(JSON.stringify($data));
  
  return false;
}

// gets a list of elements in regions
function interface_tabs_admin_get_regions(){
  var $data = new Array();
  // get a list of all tab content
  // calling it regions for now, will be actual regions when this supports multiple regions per tab
  var $order = new Array();
  $('.interface_tabs').children().not('.add_new').each(function(index, element){
    $order.push($(this).find('span').attr('name'));
  });
  
  // loop through the tab order to get the machine name for all elements in each region
  for(var $i = 0; $i < $order.length; $i++){
    $data[$i] = new Array();
    $('.' + $order[$i]).find('.machine_name').each(function(index){
      $data[$i][index] = $(this).text();
    });
  }
  
  return $data;
}


// saves the tab order for a set of tabs
function interface_tabs_admin_save_tab_order($tabs){
  Drupal.attachBehaviors($('#interface-tab-edit-form'));
  $('#interface-tab-edit-form').ajaxSubmit({
    url: Drupal.settings.interface_tab_admin_save,
    success: function(responseText, statusText, xhr, $form){
      $('#interface-tab-edit-form').parent().find('SPAN').show();
      $('#interface-tab-edit-form').remove();
      // console.log(responseText);
    }
  });
}

// returns a list of tabs, in order
function interface_tabs_admin_get_tab_order(){
  var $tabs = new Array();
  $('.interface_tabs').children().not('.add_new').each(function(index, element){
    $tabs[index] = $(this).find('SPAN').text();
  });
  return $tabs;
}

// save reordered tabs. this is now a general save function for all reordering operations, including tabs and elements.
function interface_reordered_tabs(event, ui){
  
  // the elements associated with each tab are now out of order
  // we need to retrieve a list of all the tab names
  var $order = new Array();
  $('.interface_tabs').children().not('.add_new').each(function(index, element){
    $order.push($(this).find('span').attr('name'));
  });
  
  // AND change the association with each one
  $('.interface_tabs').children().not('.add_new').each(function(index, item){
    $(this).find('SPAN').attr('name', $order[index]);
  });
  
  // now save the interface
  // set the name to the value of the current tab, only allow this to be set once at a time
  if($('#interface-tab-edit-form').length == 0 || 1 == 1){
   
    // console.log('Saving ...');
    // hide the contents of the field 
    $('#interface_admin_scratch').hide().append(Drupal.settings.interfaceTabEditForm);
    
    // make the form submit
    $('#interface-tab-edit-form').submit(interface_tabs_admin_tab_edit_form_submit);

    Drupal.attachBehaviors($('#interface-tab-edit-form'));
    
    $('#interface-tab-edit-form').submit();
    
  }
  
}

})(jQuery);
