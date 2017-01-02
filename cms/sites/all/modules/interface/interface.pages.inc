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
  $entity = entity_get_info($entity_type);
  $bundle_type = $bundle->type;
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
  );
  
  // get the admin path for each item
  $admin_path = $bundle['admin']['real path'];
  
  // iterate around each view mode for the entity to all the bundle to have an interface for each one
  foreach ($displays as $item => $options) { 
    if ($bundle['interface'][$item] != 'none') {
      
      // TODO: make sure the edit buttons go to the right edit form
      // will need to add stuff about the display mode to get it right
      $operations = l(t('Edit'), $admin_path . '/interface/edit');
      $operations .= ' | ';
      $operations .= l(t('Delete'), 'delete');
    
      $table['#rows'][] = array(
        $options['label'],
        $item,
        $bundle['interface'][$item],
        $operations,
      );
    }
  }
  $form['existing'] = $table;
  
  // display form controls for adding an interface
  $form['next_form'] = array(
    '#markup' => t('<p>Use this form to add new interfaces to this entity.</p>'),
  );
  $display_opts = array();
  $entity_info = entity_get_info($entity_type);
  
  foreach ($entity_info['view modes'] as $item => $values) {
    $display_opts[$item] = $item;
  }
  
  // TODO: exclude view modes that already have interfaces from the view mode selector
  // TODO: format the interface creation form as a single row
  // add hidden fields for the entity and the bundle, to save us from having to look them up later
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
    

  // TODO: get rid of everything below this line
  // keeping it as an example for the time being
  
  
  // TODO: get the callback for editing the interface from the plug in
  // the interface will be a callback that is used to place the elements of the entity
  $entity_name = $bundle->name;
  
  // TODO: get information about the bundle
  // copied from field_ui
  $bundle = field_extract_bundle($entity_type, $bundle);
  field_ui_inactive_message($entity_type, $bundle);
  $admin_path = _field_ui_bundle_admin_path($entity_type, $bundle);
  
  // When displaying the form, make sure the list of fields is up-to-date.
  if (empty($form_state['post'])) {
    field_info_cache_clear();
  }

  // Gather bundle information.
  $instances = field_info_instances($entity_type, $bundle);
  $field_types = field_info_field_types();
  $widget_types = field_info_widget_types();

  $extra_fields = field_info_extra_fields($entity_type, $bundle, 'form');

  $form += array(
    '#entity_type' => $entity_type,
    '#bundle' => $bundle,
    '#fields' => array_keys($instances),
    '#extra' => array_keys($extra_fields),
  );
  
  // TODO: create the interface selector
  // should create a DIV for selecting the interface style
  // should allow people to choose between TABS, REGIONS and a TABLE
  $form['interface']['selector'] = interface_selector($entity_name);

  
  $table = array(
    '#type' => 'field_ui_table',
    '#tree' => TRUE,
    '#header' => array(
      t('Label'),
      t('Weight'),
      t('Parent'),
      t('Name'),
      t('Field'),
      t('Widget'),
      array('data' => t('Operations'), 'colspan' => 2),
    ),
    '#parent_options' => array(),
    '#regions' => array(
      'main' => array('message' => t('No fields are present yet.')),
      'add_new' => array('title' => '&nbsp;'),
    ),
    '#attributes' => array(
      'class' => array('field-ui-overview'),
      'id' => 'field-overview',
    ),
  );

  // Fields.
  // TODO: change this from a table to something more representational
  // should display a draggable div for most types
  // need to remove various options for each field
  foreach ($instances as $name => $instance) {
    $field = field_info_field($instance['field_name']);
    $admin_field_path = $admin_path . '/fields/' . $instance['field_name'];
    $table[$name] = array(
      '#attributes' => array('class' => array('draggable', 'tabledrag-leaf')),
      '#row_type' => 'field',
      '#region_callback' => 'field_ui_field_overview_row_region',
      'label' => array(
        '#markup' => check_plain($instance['label']),
      ),
      'weight' => array(
        '#type' => 'textfield',
        '#title' => t('Weight for @title', array('@title' => $instance['label'])),
        '#title_display' => 'invisible',
        '#default_value' => $instance['widget']['weight'],
        '#size' => 3,
        '#attributes' => array('class' => array('field-weight')),
       ),
      'parent_wrapper' => array(
        'parent' => array(
          '#type' => 'select',
          '#title' => t('Parent for @title', array('@title' => $instance['label'])),
          '#title_display' => 'invisible',
          '#options' => $table['#parent_options'],
          '#empty_value' => '',
          '#attributes' => array('class' => array('field-parent')),
          '#parents' => array('fields', $name, 'parent'),
        ),
        'hidden_name' => array(
          '#type' => 'hidden',
          '#default_value' => $name,
          '#attributes' => array('class' => array('field-name')),
        ),
      ),
      'field_name' => array(
        '#markup' => $instance['field_name'],
      ),
      'type' => array(
        '#type' => 'link',
        '#title' => check_plain(t($field_types[$field['type']]['label'])),
        '#href' => $admin_field_path . '/field-settings',
        '#options' => array('attributes' => array('title' => t('Edit field settings.'))),
      ),
      'widget_type' => array(
        '#type' => 'link',
        '#title' => check_plain(t($widget_types[$instance['widget']['type']]['label'])),
        '#href' => $admin_field_path . '/widget-type',
        '#options' => array('attributes' => array('title' => t('Change widget type.'))),
      ),
      'edit' => array(
        '#type' => 'link',
        '#title' => t('edit'),
        '#href' => $admin_field_path,
        '#options' => array('attributes' => array('title' => t('Edit instance settings.'))),
      ),
      'delete' => array(
        '#type' => 'link',
        '#title' => t('delete'),
        '#href' => $admin_field_path . '/delete',
        '#options' => array('attributes' => array('title' => t('Delete instance.'))),
      ),
    );

    if (!empty($instance['locked'])) {
      $table[$name]['edit'] = array('#value' => t('Locked'));
      $table[$name]['delete'] = array();
      $table[$name]['#attributes']['class'][] = 'menu-disabled';
    }
  }

  // Non-field elements.
  foreach ($extra_fields as $name => $extra_field) {
    $table[$name] = array(
      '#attributes' => array('class' => array('draggable', 'tabledrag-leaf')),
      '#row_type' => 'extra_field',
      '#region_callback' => 'field_ui_field_overview_row_region',
      'label' => array(
        '#markup' => check_plain($extra_field['label']),
      ),
      'weight' => array(
        '#type' => 'textfield',
        '#default_value' => $extra_field['weight'],
        '#size' => 3,
        '#attributes' => array('class' => array('field-weight')),
        '#title_display' => 'invisible',
        '#title' => t('Weight for @title', array('@title' => $extra_field['label'])),
      ),
      'parent_wrapper' => array(
        'parent' => array(
          '#type' => 'select',
          '#title' => t('Parent for @title', array('@title' => $extra_field['label'])),
          '#title_display' => 'invisible',
          '#options' => $table['#parent_options'],
          '#empty_value' => '',
          '#attributes' => array('class' => array('field-parent')),
          '#parents' => array('fields', $name, 'parent'),
        ),
        'hidden_name' => array(
          '#type' => 'hidden',
          '#default_value' => $name,
          '#attributes' => array('class' => array('field-name')),
        ),
      ),
      'field_name' => array(
        '#markup' => $name,
      ),
      'type' => array(
        '#markup' => isset($extra_field['description']) ? $extra_field['description'] : '',
        '#cell_attributes' => array('colspan' => 2),
      ),
      'edit' => array(
        '#markup' => isset($extra_field['edit']) ? $extra_field['edit'] : '',
      ),
      'delete' => array(
        '#markup' => isset($extra_field['delete']) ? $extra_field['delete'] : '',
      ),
    );
  }

  $form['fields'] = $table;

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array('#type' => 'submit', '#value' => t('Save'));

  $form['#attached']['css'][] = drupal_get_path('module', 'field_ui') . '/field_ui.css';
  $form['#attached']['js'][] = drupal_get_path('module', 'field_ui') . '/field_ui.js';

  // Add settings for the update selects behavior.
  $js_fields = array();

  $form['#attached']['js'][] = array(
    'type' => 'setting',
    'data' => array('fields' => $js_fields, 'fieldWidgetTypes' => field_ui_widget_type_options()),
  );

  // Add tabledrag behavior.
  $form['#attached']['drupal_add_tabledrag'][] = array('field-overview', 'order', 'sibling', 'field-weight');
  $form['#attached']['drupal_add_tabledrag'][] = array('field-overview', 'match', 'parent', 'field-parent', 'field-parent', 'field-name');
  
  // TODO: add javascript for drag and drop
  
  // TODO: add fields for sorting
  
  // TODO: add regions, tabs and wrappers
  
  // dpm($form);
  
  return $form;
}
