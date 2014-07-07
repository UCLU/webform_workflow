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

  $form['#node'] = $node;
  $form_state['node'] = $node;

  $form['enable_workflow'] = array(
    '#type' => 'checkbox',
    '#title' => 'Enable workflow',
    '#default_value' => webform_workflow_is_enabled($node),
  );

  $form['states'] = array(
    '#type' => 'fieldset',
    '#prefix' => '<div id="states-fieldset-wrapper">',
    '#suffix' => '</div>',
    '#theme' => 'webform_workflow_admin_form',
    '#tree' => TRUE,
  );

  $items = webform_workflow_get_available_states($node);
  if (!isset($form_state['num_states'])) {
    $form_state['num_states'] = (count($items) + 1);
  }

  while (count($items) < $form_state['num_states']) {
    $items[] = webform_workflow_dummy_state(count($items));
  }

  $drupal_roles = user_roles(TRUE);
  $drupal_roles[WEBFORM_WORKFLOW_ORIGINAL_AUTHOR] = 'original submitter';

  $ops = array('view', 'edit', 'from', 'to');

  $role_checkboxes = array(
    '#type' => 'checkboxes',
    '#options' => $drupal_roles,
    '#default_value' => array(),
  );

  foreach ($items as $item) {
    $state = webform_workflow_get_state($item['wsid']);
    foreach ($ops as $op) {
      ${$op . '_roles'} = $role_checkboxes;
      if (!empty($state[$op . '_permissions'])) {
        ${$op . '_roles'}['#default_value'] = $state[$op . '_permissions'];
      }
    }

    $form['states']['items'][] = array(
      'wsid' => array(
        '#type' => 'hidden',
        '#value' => $item['wsid'],
      ),
      'label' => array(
        '#type' => 'textfield',
        '#default_value' => $item['label'],
        '#size' => 16,
      ),
      'view_permissions' => $view_roles,
      'edit_permissions' => $edit_roles,
      'from_permissions' => $from_roles,
      'to_permissions' => $to_roles,
      'notify_users' => array(
        '#type' => 'textarea',
        '#default_value' => webform_workflow_get_user_text($item['notify_users']),
        '#rows' => 3,
        '#cols' => 32,
      ),
      'weight' => array(
        '#type' => 'weight',
        '#title' => t('Weight'),
        '#default_value' => $item['weight'],
        '#title_display' => 'invisible',
        '#attributes' => array('class' => array('state-item-weight')),
      ),
    );
  }

  $form['states']['add_another'] = array(
    '#type' => 'submit',
    '#value' => t('Add one more'),
    '#submit' => array('webform_workflow_add_another_submit'),

    // See the examples in ajax_example.module for more details on the
    // properties of #ajax.
    '#ajax' => array(
      'callback' => 'webform_workflow_add_more_callback',
      'wrapper' => 'states-fieldset-wrapper',
    ),
  );

  $workflow = isset($node->webform_workflow) ? $node->webform_workflow : FALSE;
  $emails = $workflow ? $workflow->data['emails'] : webform_workflow_get_default_email();
  $form['emails'] = array(
    '#type' => 'fieldset',
    '#title' => 'State Transition Notification Emails',
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
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
function webform_workflow_config_validate($form, &$form_state) {
  $enabled = (bool) $form_state['values']['enable_workflow'];
  if (!$enabled) {
    // Don't validate if not enabled.
    return;
  }

  foreach ($form_state['values']['items'] as $key => $item) {
    $label = trim($item['label']);
    if (!empty($label)) {
      webform_workflow_get_users($item['notify_users'], $user_errors);
      if (!empty($user_errors)) {
        form_set_error("workflow][states][items][$key][notify_users",
          t('Invalid username or email %user for %state state',
            array('%user' => implode(', ', $user_errors), '%state' => $label)));
      }
    }
  }
}

/**
 * Submit handler for the workflow fieldset.
 *
 * Writes the provided workflows to the database.
 */
function webform_workflow_config_submit($form, $form_state) {
  $node = $form_state['node'];

  foreach ($form_state['values']['states']['items'] as $item) {
    $label = trim($item['label']);
    if (!empty($label)) {
      $state = array(
        'label' => $label,
        'view_permissions' => array_keys(array_filter($item['view_permissions'])),
        'edit_permissions' => array_keys(array_filter($item['edit_permissions'])),
        'from_permissions' => array_keys(array_filter($item['from_permissions'])),
        'to_permissions' => array_keys(array_filter($item['to_permissions'])),
        'notify_users' => webform_workflow_get_users($item['notify_users']),
        'weight' => $item['weight'],
        'wsid' => $item['wsid'],
      );

      webform_workflow_write_state($node->nid, $state);
    }
    elseif ($wsid = $item['wsid']) {
      webform_workflow_delete_state($wsid);
    }
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

/**
 * Get array of emails from array of user uids or emails.
 */
function webform_workflow_get_user_emails($notify_users) {
  $emails = array();
  foreach ($notify_users as $u) {
    if (is_numeric($u)) {
      $user = user_load($u);
      $emails[] = $user->mail;
    }
    else {
      $emails[] = $u;
    }
  }
  return $emails;
}
