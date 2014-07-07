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
  $form_state['state'] = $state;

  $drupal_roles = user_roles(TRUE);
  $drupal_roles[WEBFORM_WORKFLOW_ORIGINAL_AUTHOR] = 'original submitter';

  $ops = array(
    'view' => t('View'),
    'edit' => t('Edit'),
    'from' => t('Change from'),
    'to' => t('Change to'),
  );

  $role_checkboxes = array(
    '#type' => 'select',
    '#multiple' => TRUE,
    '#options' => $drupal_roles,
    '#default_value' => array(),
  );

  $form['label'] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#default_value' => $state->label,
    '#size' => 16,
  );

  $form['permissions'] = array(
    '#type' => 'fieldset',
    '#title' => t('Permissions'),
    '#collapsible' => TRUE,
  );

  $rows = &$form['permissions']['#rows'];

  foreach ($ops as $op => $label) {
    $form['permissions'][$op . '_permissions'] = $role_checkboxes;
    $form['permissions'][$op . '_permissions']['#title'] = $label;
    if (!empty($state->data['permissions'][$op])) {
      $form['permissions'][$op . '_permissions']['#default_value'] = $state->data['permissions'][$op];
    }
  }

  $form['notify_users'] = array(
    '#type' => 'textarea',
    '#title' => t('Notification e-mail addresses'),
    '#default_value' => !empty($state->data['notify_users']) ? webform_workflow_get_user_text($state->data['notify_users']) : '',
    '#rows' => 3,
    '#cols' => 32,
  );

  $form['buttons'] = array('#type' => 'actions');
  $form['buttons']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

/**
 * Submit callback for adding or editing a state.
 */
function webform_workflow_state_form_submit($form, &$form_state) {

}
