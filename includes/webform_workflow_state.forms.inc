<?php
/**
 * @file
 * State-related forms.
 */

/**
 * Form for editing or adding a state.
 */
function webform_workflow_state_form($form, &$form_state, $node, $state = NULL) {

  if (empty($state)) {
    $state = entity_create('webform_workflow_state', array(
      'nid' => $node->nid,
    ));
  }

  form_load_include($form_state, 'inc', 'webform_workflow', 'includes/webform_workflow_state.forms');

  $form_state['node'] = $node;

  webform_workflow_state_form_attach($form, $form_state, $state);

  $form['buttons'] = array('#type' => 'actions');
  $form['buttons']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

/**
 * Attach form elements for a workflow state form.
 */
function webform_workflow_state_form_attach(&$form, &$form_state, $state) {
  $form_state['state'] = $state;

  $ops = array(
    'view' => t('View submission'),
    'edit' => t('Edit submission'),
    'from' => t('Change from this state'),
    'to' => t('Change to this state'),
  );

  $form['label'] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#default_value' => $state->label,
    '#size' => 16,
    '#required' => TRUE,
  );

  $form['data'] = array(
    '#tree' => TRUE,
  );
  $form['data']['permissions'] = array(
    '#type' => 'fieldset',
    '#title' => t('Permissions'),
    '#tree' => TRUE,
    '#attributes' => array('class' => array('webform-workflow-state-permissions')),
  );
  $drupal_roles = user_roles(TRUE);
  $drupal_roles[WEBFORM_WORKFLOW_ORIGINAL_SUBMITTER] = 'original submitter';
  $role_checkboxes = array(
    '#type' => 'select',
    '#multiple' => TRUE,
    '#options' => $drupal_roles,
  );

  foreach ($ops as $op => $label) {
    $form['data']['permissions'][$op] = $role_checkboxes + array(
      '#title' => $label,
      '#default_value' => $state->data['permissions'][$op],
    );
  }

  $form['data']['notify_users'] = array(
    '#type' => 'textarea',
    '#title' => t('Notification e-mail addresses'),
    '#default_value' => webform_workflow_get_user_text($state->data['notify_users']),
    '#rows' => 3,
    '#cols' => 32,
    '#element_validate' => array('webform_workflow_state_validate_notify_users'),
  );

  $form['#attached']['css'][] = drupal_get_path('module', 'webform_workflow') . '/includes/webform_workflow.css';
}

/**
 * Submit callback for the webform workflow state form.
 */
function webform_workflow_state_form_submit($form, &$form_state) {
  $values = $form_state['values'];
  $state = $form_state['state'];
  $instances = field_info_instances('webform_workflow_state', 'webform_workflow_state');

  $values_excluding_fields = array_diff_key($values, $instances);
  foreach ($values_excluding_fields as $property => $value) {
    $state->$property = $value;
  }

  if ($instances) {
    field_attach_submit('webform_workflow_state', $state, $form, $form_state);
  }
  entity_get_controller('webform_workflow_state')->save($state);
}

/**
 * Validate the 'notify_users' element.
 */
function webform_workflow_state_validate_notify_users($element, &$form_state) {
  $value = $element['#value'];
  if (!empty(trim($value))) {
    webform_workflow_get_users($value, $user_errors);
    if (!empty($user_errors)) {
      form_error($element, t('Invalid username or email: %user', array('%user' => implode(', ', $user_errors))));
    }
  }
}

/**
 * Get user uids or emails from form entry text.
 *
 * Sets $user_errors for any invalid users
 * Returns an array of valid uid (numeric) or emails (string)
 */
function webform_workflow_get_users($notify_users_text, &$user_errors = NULL) {
  $notify_users_text = trim($notify_users_text);
  $users = preg_split('/[\s;,]+/', $notify_users_text);
  $valid_users = array();
  $user_errors = array();
  foreach ($users as $user_text) {
    if (valid_email_address($user_text)) {
      $valid_users[] = $user_text;
    }
    elseif ($user = user_load_by_name($user_text)) {
      $valid_users[] = $user->uid;
    }
    else {
      $user_errors[] = $user_text;
    }
  }
  return $valid_users;
}
