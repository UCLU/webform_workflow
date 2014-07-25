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
    '#title' => t('Enable workflow'),
    '#description' => t('Enable workflow for this form.'),
    '#default_value' => webform_workflow_is_enabled($node),
  );

  $form['states'] = array(
    '#type' => 'container',
  );
  field_attach_form('node', $node, $form['states'], $form_state, NULL, array('field_name' => 'webform_workflow_states'));

  $form['require_log'] = array(
    '#type' => 'checkbox',
    '#title' => t('Require a log message'),
    '#description' => t('Set whether users must enter a log message whenever they change the state of a submission.'),
    '#default_value' => isset($node->webform_workflow) && !empty($node->webform_workflow->data['require_log']),
  );

  $emails = isset($node->webform_workflow) ? $node->webform_workflow->data['emails'] : webform_workflow_get_default_email();

  $form['emails'] = array(
    '#type' => 'fieldset',
    '#title' => t('State transition notification e-mails'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#tree' => TRUE,
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
  field_attach_submit('node', $node, $form['states'], $form_state);

  $data_array = array(
    'emails' => $form_state['values']['emails'],
    'require_log' => $form_state['values']['require_log'],
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
  $group = ($state->wsid) && count($available_states) > 1;
  $new_state_colors = array();
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
      if ($group) {
        $key = $seen_current ? t('Later states') : t('Earlier states');
        $new_state_options[$key][$new_state->wsid] = $new_state->label;
      }
      else {
        $new_state_options[$new_state->wsid] = $new_state->label;
      }
      if ($new_state->color) {
        $new_state_colors['state-' . $new_state->wsid] = $new_state->color;
      }
    }
  }

  $form['workflow'] = array(
    '#type' => 'fieldset',
    '#title' => t('Workflow'),
  );

  $form['workflow']['current_state'] = array(
    '#type' => 'item',
    '#title' => t('Current state'),
    '#markup' => theme('webform_workflow_state', array('state' => $state)),
  );

  $log = db_select('webform_workflow_log', 'wwl')
    ->fields('wwl')
    ->condition('sid', $submission->sid)
    ->orderBy('id', 'DESC')
    ->range(0, 1)
    ->execute()
    ->fetch();
  if ($log) {
    $account = $log->uid ? user_load($log->uid) : FALSE;
    $log_string = t('@time: %old &rarr; %new (changed by !user)', array(
      '@time' => format_date($log->timestamp, 'short'),
      '!user' => theme('username', array('account' => $account)),
      '%old' => $log->old_state_label,
      '%new' => $log->new_state_label,
    ));
    if ($log->message) {
      $log_string .= '<blockquote class="webform-workflow-log-message">'
        . check_plain($log->message) . '</blockquote>';
    }
    $form['workflow']['log'] = array(
      '#type' => 'item',
      '#title' => t('Last state change'),
      '#markup' => $log_string,
      '#weight' => 50,
    );
  }

  if (!$new_state_options) {
    return $form;
  }

  $form_state['submission'] = $submission;

  $form['workflow']['new_state'] = array(
    '#title' => t('New state'),
    '#type' => 'select',
    '#options' => $new_state_options,
    '#required' => TRUE,
    '#title_display' => 'before',
    '#weight' => 20,
  );

  if ($new_state_colors) {
    $form['workflow']['new_state']['#attributes']['class'] = array('webform-workflow-state-select');
    $form['workflow']['new_state']['#attached']['js'] = array(
      drupal_get_path('module', 'webform_workflow') . '/includes/webform_workflow.js',
      array(
        'data' => array('webformWorkflow' => array('stateColors' => $new_state_colors)),
        'type' => 'setting',
      ),
    );
  }

  $new_state_trigger = array(
    ':input[name="new_state"]' => array('!value' => ''),
  );
  $form['workflow']['message'] = array(
    '#type' => 'textfield',
    '#title' => t('Log message'),
    '#description' => t('Describe why you are making this change.'),
    '#states' => array('visible' => $new_state_trigger),
    '#required' => !empty($node->webform_workflow->data['require_log']),
    '#weight' => 30,
  );
  $form['workflow']['buttons'] = array(
    '#type' => 'actions',
    '#states' => array('visible' => $new_state_trigger),
    '#weight' => 40,
  );
  $form['workflow']['buttons']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Change state'),
  );
  $destination = drupal_get_destination();
  $form['workflow']['buttons']['cancel'] = array(
    '#type' => 'link',
    '#title' => t('Cancel'),
    '#href' => current_path(),
  );
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
