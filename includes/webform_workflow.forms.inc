<?php
/**
 * @file
 * Forms for the Webform Workflow module.
 */

/**
 * Configuration form.
 */
function webform_workflow_config_form($form, &$form_state, $node) {

  form_load_include($form_state, 'inc', 'webform_workflow', 'includes/webform_workflow.forms');

  $form_state['node'] = $node;

  $form['enable_workflow'] = array(
    '#type' => 'checkbox',
    '#title' => 'Enable workflow',
    '#default_value' => webform_workflow_is_enabled($node),
  );

  $form['states'] = array(
    '#type' => 'container',
    '#states' => array(
      'visible' => array(
        'input[name="enable_workflow"]' => array('checked' => TRUE),
      ),
    ),
  );
  field_attach_form('node', $node, $form['states'], $form_state, NULL, array('field_name' => 'field_ww_states'));

  $emails = isset($node->webform_workflow) ? $node->webform_workflow->data['emails'] : webform_workflow_get_default_email();

  $form['emails'] = array(
    '#type' => 'fieldset',
    '#title' => t('State transition notification e-mails'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#tree' => TRUE,
    '#states' => array(
      'visible' => array(
        'input[name="enable_workflow"]' => array('checked' => TRUE),
      ),
    ),
  );
  $form['emails']['subject'] = array(
    '#type' => 'textfield',
    '#title' => 'Message Subject',
    '#default_value' => $emails['subject'],
    '#size' => 80,
  );
  $form['emails']['body'] = array(
    '#type' => 'textarea',
    '#title' => 'Message Body',
    '#default_value' => $emails['body'],
    '#rows' => 5,
    '#cols' => 80,
    '#description' => theme('webform_token_help', array('groups' => array('node', 'submission'))),
  );

  $form['buttons'] = array('#type' => 'actions');
  $form['buttons']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

/**
 * Validation handler for the workflow fieldset.
 *
 * Ensures valid inputs otherwise sets form errors.
 */
function webform_workflow_config_form_validate($form, &$form_state) {
  $node = $form_state['node'];
  if ($form_state['values']['enable_workflow']) {
    field_attach_form_validate('node', $node, $form['states'], $form_state);
  }
}

/**
 * Submit handler for the workflow fieldset.
 *
 * Writes the provided workflows to the database.
 */
function webform_workflow_config_form_submit($form, &$form_state) {
  $node = $form_state['node'];
  if ($form_state['values']['enable_workflow']) {
    field_attach_submit('node', $node, $form['states'], $form_state);
  }

  $data_array = array(
    'emails' => $form_state['values']['emails'],
  );
  db_merge('webform_workflow')
    ->key(array('nid' => $node->nid))
    ->fields(array(
      'workflow' => $form_state['values']['enable_workflow'] ? 1 : 0,
      'data' => serialize($data_array),
    ))
    ->execute();
}
