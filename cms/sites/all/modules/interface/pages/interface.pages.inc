<?php 
/**
 @file Interface Page Handling
*/

/**
 * Administrative form for selecting which entities will be under interface control
 * @param array $form The form we are editing
 * @param array $form_state The form state
 */
function interface_admin_form($form, &$form_state) {
  
  // TODO: break this out into interface_ui
  // get the form set up
  // TODO: determine if we really need the field ui, not sure if it is necessary in this instance
  require_once drupal_get_path('module', 'field_ui') . '/field_ui.admin.inc';
  $form = array();
  $options = array();
  
  // create a list of all entity types
  foreach (entity_get_info() as $entity_type => $entity_info) {
    $options[$entity_type] = $entity_info['label'];
  }
  
  // output it as a set of checkboxes
  $form['interface'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Enable interface for all bundles within the following entities'),
    '#options' => $options,
    '#default_value' => variable_get('interface_entities'),
  );
  
  // add submit buttons
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array('#type' => 'submit', '#value' => t('Save'));
  
  return $form;
}

function interface_admin_form_submit($form, &$form_state) {
  $form_values = $form_state['values'];

  // store the enabled settings as a drupal variable
  variable_set('interface_entities', $form_values['interface']);
  
  // TODO: need to clear the cache before saving, this can change some menu values
  
}


/**
 * Displays the form for selecting entity displays for editing
 * @param array $form The form we are editing
 * @param array $form_state The form state
 * @param string $entity_type The entity type we are editing
 * @param array $bundle The bundle we are editing
 */
function interface_select_form($form, &$form_state, $entity_type, $bundle) {

  // TODO: break this out into interface_ui
    
  // set up the entity and bundle for processing
  // IMPORTANT: a bundle should be an object, some modules will load the bundle as such through a URL parameter and a entity_load function
  // when it's not, let's load the bundle into 
  // so, what we have been building around is the default entity_load information from the entity api
  // what we need to do is duplicate that somehow for entities that don't use entity api (the module)
  // it will support these fields
  // --> type
  // --> name            -> label
  // --> description
  // --> custom
  
  // TODO: remove this and put it in the save feature for the interface
  drupal_static_reset('entity_get_info');
  cache_clear_all('entity_info', 'cache');
  entity_info_cache_clear();
  
  $entity = entity_get_info($entity_type);
  
  if(!is_object($bundle)){
    $bundle = $entity['bundles'][$bundle];
    $bundle_type = $bundle->type;
  } else {
    $bundle_type = $bundle->type;
  }
  
  // we need a little more information about the bundle at this point
  // override it with information from the entity
  $bundle = $entity['bundles'][$bundle_type];
  drupal_set_title(t('Manage interfaces for @label', array('@label' => $bundle['label'])));
  
  // PREPARE THE FORM
  // get data for laying out the defined interfaces for this entity / bundle
  // get a list of the view modes for the entity
  $displays = $entity['view modes'];
  
  // include the edit form, it will be open to interfacing
  $displays['edit_screen'] = array(
    'label'  => 'Edit Screen',
    'custom settings'  => FALSE,
  );
    // get a list of all installed interface plugins
  $plugins_data = interface_get_plugins();
  $plugins = array();
  foreach ($plugins_data as $key => $val) {
    $plugins[$key] = $val['name'];
  }
  $plugins['none'] = 'No interface defined';
  
  // CREATE THE FORM
  // create the form for displaying view modes and interface options
  $form = array();
  $form['intro'] = array(
    '#markup' => t('<p>Interface is enabled for this entity. The following view modes have interfaces defined for them. Define new interfaces using the form below.</p>'),
  );
  
  // display all the view modes for this entity
  // there can be one interface for each view mode on each bundle
  $table = array(
    '#theme' => 'table',
    '#header' => array(
      t('Display'),
      t('ID'),
      t('Interface'),
      t('Options'),
    ),
    '#rows' => array(),
    '#empty' => t('There are no interfaces defined for @label', array('@label' => $bundle['label'])),
  );
  
  // get the admin path for each item
  $admin_path = $bundle['admin']['real path'];
  
  // stores a list of view modes already in use so they can be hidden in the creation form
  $used_modes = array();
  
  // iterate around each view mode for the entity to all the bundle to have an interface for each one
  foreach ($displays as $item => $options) { 
    if ($bundle['interface'][$item] !== 'none') {
      $used_modes[] = $item;
      $operations = l(t('Edit'), $admin_path . '/interface/edit/' . $item);
      $operations .= ' | ';
      $operations .= l(t('Delete'), $admin_path . '/interface/delete/' . $item);
      $table['#rows'][] = array(
        $options['label'],
        $item,
        $bundle['interface'][$item],
        $operations,
      );
    }
  }

  $form['existing'] = $table;
  
  $check = array_search('full', $used_modes);
  
  $display_opts = array();
  $entity_info = entity_get_info($entity_type);
  
  // loop around the view_modes to populate the interface creation form
  // hide the view modes that are already in use
  foreach ($entity_info['view modes'] as $item => $values) {
    if(array_search($item, $used_modes) === FALSE){
      $display_opts[$item] = $item;
    }
  }
  
  // TODO: exclude view modes that already have interfaces from the view mode selector
  // if there are no more view modes to be defined, hide this form entirely
  // TODO: format the interface creation form as a single row
  // add hidden fields for the entity and the bundle, to save us from having to look them up later
  if(sizeof($display_opts) > 0){
    
    // display form controls for adding an interface
    $form['next_form'] = array(
      '#markup' => t('<p>Use this form to add new interfaces to this entity.</p>'),
    );
    
    $form['selector']['entity'] = array(
        '#type' => 'hidden',
        '#value' => $entity_type,
    );
    $form['selector']['bundle'] = array(
        '#type' => 'hidden',
        '#value' => $bundle_type,
    );
    $form['selector']['view_mode'] = array(
        '#type' => 'select',
        '#title' => 'View Mode',
        '#options' => $display_opts,
        '#description' => t('Select the view mode to edit'),
    );
    $form['selector']['plugins'] = array(
        '#type' => 'select',
        '#title' => 'Plugin',
        '#options' => $plugins,
        '#description' => t('Select the type of interface you want to build'),
    );
    
    $form['selector']['actions'] = array('#type' => 'actions');
    $form['selector']['actions']['submit'] = array('#type' => 'submit', '#value' => t('Create New Interface'));
    
  }
  
  return $form;
  
}

/**
 * Submit function for the interface select form
 */
function interface_select_form_submit($form, &$form_state) {
  
  // clean up the form a little to allow values to be saved
  $values = $form_state['values'];
  $values['plugin'] = $values['plugins'];
  
  // store the entity, bundle, view mode and plug in, along with a primary key (which I think is necessary)
  drupal_write_record('interface', $values);
  
  // push the user to the edit form
  $action = str_replace(base_path(), '', $form['#action']);
  drupal_goto($action . '/edit/' . $values['view_mode']);
  
}

/**
* Delete an interface
*
*/
function interface_delete($form, &$form_state, $entity_type, $bundle_arg, $view_mode){

  // get the current interface, using this to get the plugin
  $interface = db_select('interface', 'i')
  ->fields('i')
  ->condition('entity', $entity_type)
  ->condition('bundle', $bundle_arg->type)
  ->condition('view_mode', $view_mode)
  ->execute()
  ->fetchAssoc();

  // set the question
  $question = 'You are about to delete the %type interface for %bundle_arg, view mode %view_mode.';
  // set the description
  $description = 'Are you sure you want to delete this interface?';
  // other stuff
  $yes = 'Delete this Interface';
  $no = 'Cancel';

  // construct path by removing the view mode and the DELETE operation from the current path
  $path = split('/', $_GET['q']);
  // remove delete and the item following it
  $item = array_search('delete', $path);
  unset($path[$item]);
  unset($path[$item+1]);
  // reassemble the path
  $path = implode('/', $path);
  
  $form = array();
  $output = confirm_form($form,
    t($question, array('%type' => $interface['plugin'], '%bundle_arg' => $bundle_arg->name, '%view_mode' => $view_mode)),
    $path,
    t($description),
    t($yes), t($no)
  );
  $output['path'] = array(
    '#type'		  => 'hidden',
    '#value'		=> $path,
  );
  $output['iid'] = array(
    '#type'		  => 'hidden',
    '#value'		=> $interface['iid'],
  );
  
  return $output;
}
/**
 * Submit handler for interface delete
 */
function interface_delete_submit($form, &$form_state){
  // deletes the interface with the selected iid
  
  db_delete('interface')
    ->condition('iid', $form_state['input']['iid'])
    ->execute();
  $form_state['redirect'] = $form_state['input']['path'];
  interface_clear_caches();
}

/**
 * Displays the form for editing an interface
 * @param array $form The form we are editing
 * @param array $form_state The form state
 * @param string $entity_type The entity type we are editing
 * @param array $bundle The bundle we are editing
 */
function interface_edit($form_id, $entity_type, $bundle, $view_mode = '') {

  require_once drupal_get_path('module', 'field_ui') . '/field_ui.admin.inc';
  
  // DEBUG: let's see what's getting passed
  // dpm($entity_type);
  // dpm($bundle);
  // dpm($view_mode); // will need to sort this
  
  // get the plug in for this entity / bundle / view mode
  $interface = db_select('interface', 'i')
    ->fields('i')
    ->condition('entity', $entity_type)
    ->condition('bundle', $bundle->type)
    // ->condition('view_mode', $view_mode)
    ->execute()
    ->fetchAssoc();
    
  $plugin = interface_get_plugins($interface['plugin']);
  $interface_data = drupal_json_decode($interface['interface_data']);
  
  // load any necessary inforamtion about the entity  
  $admin_path = _field_ui_bundle_admin_path($entity_type, $bundle->type);
  if (empty($form_state['post'])) {
    field_info_cache_clear();
  }
  // this gives us a list of all instances associated with the entity / bundle 
  $instances = array();
  $instances = field_info_instances($entity_type, $bundle->type);
  // this gives us all extra fields associated with the bundle. this should only be necessary when creating an interface for an entry form
  // TODO: make this only appear when editing an entry form
  // entry forms are designated by view mode = 'edit_screen'
  $extra_fields = array();
  $extra_fields = field_info_extra_fields($entity_type, $bundle->type, 'form');
  
  // let the plug in create the form and all necessary controls
  // each time we call a plugin, pass along all the information needed to goof around with the entity
  $form = $plugin[$interface['plugin']]['edit_callback']($interface['iid'], $entity_type, $bundle->type, $view_mode, $interface_data, $instances, $extra_fields);
  
  return $form;
}

// TODO: remove this if it turns out we don't need a form for the edit page
// the form shoudl really come from the plugin, regardless
/**
 * Displays the form for editing an interface
 * @param array $form The form we are editing
 * @param array $form_state The form state
 * @param string $entity_type The entity type we are editing
 * @param array $bundle The bundle we are editing
 */
function interface_edit_form($form, &$form_state, $entity_type, $bundle, $view_mode) {

  require_once drupal_get_path('module', 'field_ui') . '/field_ui.admin.inc';
  
  // DEBUG: let's see what's getting passed
  // dpm($entity_type);
  // dpm($bundle);
  // dpm($view_mode);
  
  // get the plug in for this entity / bundle / view mode
  $interface = db_select('interface', 'i')
    ->fields('i')
    ->condition('entity', $entity_type)
    ->condition('bundle', $bundle->type)
    ->execute()
    ->fetchAssoc();
  $plugin = interface_get_plugins($interface['plugin']);
  $interface_data = $interface['interface_data'];
  
  // load any necessary inforamtion about the entity  
  $admin_path = _field_ui_bundle_admin_path($entity_type, $bundle->type);
  if (empty($form_state['post'])) {
    field_info_cache_clear();
  }
  // this gives us a list of all instances associated with the entity / bundle 
  $instances = array();
  $instances = field_info_instances($entity_type, $bundle->type);
  // this gives us all extra fields associated with the bundle. this should only be necessary when creating an interface for an entry form
  // TODO: make this only appear when editing an entry form
  // entry forms are designated by view mode = 'edit_screen'
  $extra_fields = array();
  $extra_fields = field_info_extra_fields($entity_type, $bundle->type, 'form');
  
  // let the plug in create the form and all necessary controls
  // each time we call a plugin, pass along all the information needed to goof around with the entity
  $form = $plugin[$interface['plugin']]['edit_callback']($interface['iid'], $entity_type, $bundle->type, $view_mode, $interface_data, $instances, $extra_fields);
  
  return $form;

}
