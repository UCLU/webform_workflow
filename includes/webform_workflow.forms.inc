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

  $enable_workflow_trigger = array(
    'input[name="enable_workflow"]' => array('checked' => TRUE),
  );

  $form['states'] = array(
    '#type' => 'container',
    '#states' => array('visible' => $enable_workflow_trigger),
  );
  field_attach_form('node', $node, $form['states'], $form_state, NULL, array('field_name' => 'webform_workflow_states'));

  $emails = isset($node->webform_workflow) ? $node->webform_workflow->data['emails'] : webform_workflow_get_default_email();

  $form['emails'] = array(
    '#type' => 'fieldset',
    '#title' => t('State transition notification e-mails'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#tree' => TRUE,
    '#states' => array('visible' => $enable_workflow_trigger),
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

  node_save($node);
}

/**
 * Form for changing the state of an existing submission.
 */
function webform_workflow_submission_state_form($form, &$form_state, $submission) {
  global $user;

  form_load_include($form_state, 'inc', 'webform_workflow', 'includes/webform_workflow.forms');

  $node = node_load($submission->nid);
  $is_admin = node_access('update', $node);

  $state = $submission->webform_workflow_state;
  $transition_from_current = $is_admin || array_intersect(array_keys($user->roles), $state->data['permissions']['from']);
  if (!$transition_from_current) {
    return;
  }

  $new_state_options = array();
  $seen_current = FALSE;
  $available_states = webform_workflow_get_available_states($node);
  foreach ($available_states as $new_state) {
    if ($new_state->wsid == $state->wsid) {
      $seen_current = TRUE;
      continue;
    }
    if (array_intersect(array_keys($user->roles), $new_state->data['permissions']['to'])
        || $is_admin) {
      // Group the state options into 'Earlier' and 'Later'. This assumes that
      // the Entity API has provided the available states correctly sorted (this
      // does appear to work with Inline Entity Form).
      if ($state->wsid) {
        $key = $seen_current ? t('Later states') : t('Earlier states');
        $new_state_options[$key][$new_state->wsid] = $new_state->label;
      }
      else {
        $new_state_options[$new_state->wsid] = $new_state->label;
      }
    }
  }

  $form['current_state'] = array(
    '#type' => 'item',
    '#title' => t('Current state'),
    '#markup' => check_plain($state->label),
  );

  if (!$new_state_options) {
    return $form;
  }

  $form_state['submission'] = $submission;

  $form['new_state'] = array(
    '#title' => t('New state'),
    '#type' => 'select',
    '#options' => $new_state_options,
    '#required' => TRUE,
  );
  $new_state_trigger = array(
    ':input[name="new_state"]' => array('!value' => ''),
  );
  $form['message'] = array(
    '#type' => 'textfield',
    '#title' => t('Log message'),
    '#description' => t('Optionally, describe why you are making this change.'),
    '#states' => array('visible' => $new_state_trigger),
  );
  $form['buttons'] = array(
    '#type' => 'actions',
    '#states' => array('visible' => $new_state_trigger),
  );
  $form['buttons']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Change state'),
  );

  $form['#attached']['css'][] = drupal_get_path('module', 'webform_workflow') . '/includes/webform_workflow.css';
  return $form;
}

/**
 * Submit callback for changing the state of an existing submission.
 */
function webform_workflow_submission_state_form_submit($form, &$form_state){
  $submission = $form_state['submission'];
  $new_state = webform_workflow_state_load($form_state['values']['new_state']);
  $message = $form_state['values']['message'];
  webform_workflow_transition($submission, $new_state, $message);
}
