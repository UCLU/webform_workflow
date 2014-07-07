<?php
/**
 * @file
 * Admin related configuration forms 
 */

/**
 * Implements hook_form_FORM_ID_alter().
 *
 * Adds in the workflow configuration fieldset.
 */
function webform_workflow_form_webform_configure_form_alter(&$form, &$form_state, $form_id) {

  form_load_include($form_state, 'inc', 'webform_workflow', 'webform_workflow.admin');

  $nid = $form['#node']->nid;
  $form['workflow'] = array(
    '#type' => 'fieldset',
    '#title' => 'Workflow',
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#weight' => -109,
    '#tree' => TRUE,
  );

  $workflow_enabled = webform_workflow_is_enabled($nid);
  $form['workflow']['enable_workflow'] = array(
    '#type' => 'checkbox',
    '#title' => 'Enable workflow',
    '#default_value' => $workflow_enabled,
  );

  $form['workflow']['states'] = array(
    '#type' => 'fieldset',
    '#prefix' => '<div id="states-fieldset-wrapper">',
    '#suffix' => '</div>',
    '#theme' => 'webform_workflow_admin_form',
  );

  $items = webform_workflow_get_available_states($nid);
  if (!isset($form_state['num_states'])) {
    $form_state['num_states'] = (count($items) + 1);
  }

  while (count($items) < $form_state['num_states']) {
    $items[] = webform_workflow_dummy_state(count($items));
  }

  $drupal_roles = user_roles(TRUE);
  $drupal_roles['-1'] = 'original submitter';

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

    $form['workflow']['states']['items'][] = array(
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

  $form['workflow']['states']['add_another'] = array(
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

  $workflow = webform_workflow_load($nid);
  $emails = $workflow ? $workflow->data['emails'] : webform_workflow_get_default_email();
  $form['workflow']['emails'] = array(
    '#type' => 'fieldset',
    '#title' => 'State Transition Notification Emails',
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );
  $form['workflow']['emails']['subject'] = array(
    '#type' => 'textfield',
    '#title' => 'Message Subject',
    '#default_value' => $emails['subject'],
    '#size' => 80,
  );
  $form['workflow']['emails']['body'] = array(
    '#type' => 'textarea',
    '#title' => 'Message Body',
    '#default_value' => $emails['body'],
    '#rows' => 5,
    '#cols' => 80,
    '#description' => theme('webform_token_help', array('groups' => array('node', 'submission'))),
  );

  $form['#validate'][] = 'webform_workflow_config_validate';
  $form['#submit'][] = 'webform_workflow_config_submit';
}

/**
 * Validation handler for the workflow fieldset.
 *
 * Ensures valid inputs otherwise sets form errors.
 */
function webform_workflow_config_validate($form, &$form_state) {
  $values = $form_state['values']['workflow'];

  $enabled = (int)$values['enable_workflow'];
  if (!$enabled) {
    // Don't validate if not enabled.
    return;
  }

  foreach ($values['states']['items'] as $key => $item) {
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
 * Get form entry text from array of user uids or emails.
 */
function webform_workflow_get_user_text($notify_users) {
  $users = array();
  foreach ($notify_users as $u) {
    if (is_numeric($u)) {
      $user = user_load($u);
      $users[] = $user->name;
    }
    else {
      $users[] = $u;
    }
  }
  return implode("\n", $users);
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

/**
 * Submit handler for the workflow fieldset.
 *
 * Writes the provided workflows to the database.
 */
function webform_workflow_config_submit($form, $form_state) {
  $nid = $form_state['values']['nid'];
  $values = $form_state['values']['workflow'];

  foreach ($values['states']['items'] as $item) {
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

      webform_workflow_write_state($nid, $state);
    }
    elseif ($wsid = $item['wsid']) {
      webform_workflow_delete_state($wsid);
    }
  }

  $enabled = (int)$values['enable_workflow'];
  $data_array = array(
    'emails' => $values['emails'],
  );
  db_merge('webform_workflow')
    ->key(array('nid' => $nid))
    ->fields(array(
      'workflow' => $enabled,
      'data' => serialize($data_array),
    ))
    ->execute();
}

/**
 * Theme function for the state table.
 */
function theme_webform_workflow_admin_form($variables) {
  $form = $variables['element'];

  $rows = array();
  foreach (element_children($form['items']) as $key) {
    $rows[] = array(
      'data' => array(
        drupal_render($form['items'][$key]['label']),
        drupal_render($form['items'][$key]['view_permissions']),
        drupal_render($form['items'][$key]['edit_permissions']),
        drupal_render($form['items'][$key]['from_permissions']),
        drupal_render($form['items'][$key]['to_permissions']),
        drupal_render($form['items'][$key]['notify_users']),
        drupal_render($form['items'][$key]['weight']),
      ),
      'class' => array('draggable'),
    );
  }

  $table_id = 'webform-workflow-states';

  $header = array(
    t('State'),
    t('View Permissions'),
    t('Edit Permissions'),
    t('Transition From'),
    t('Transition To'),
    t('Notify users/emails'),
    t('Weight'),
  );

  $output = theme('table', array(
    'header' => $header,
    'rows' => $rows,
    'attributes' => array('id' => $table_id),
  ));
  $output .= drupal_render_children($form);

  drupal_add_tabledrag($table_id, 'order', 'sibling', 'state-item-weight');
  return $output;
}

/**
 * Ajax callback for the add another state button.
 */
function webform_workflow_add_more_callback($form, $form_state) {
  return $form['workflow']['states'];
}

/**
 * Submission callback for the add another state button.
 */
function webform_workflow_add_another_submit($form, &$form_state) {
  $form_state['num_states']++;
  $form_state['rebuild'] = TRUE;
}
