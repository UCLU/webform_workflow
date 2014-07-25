<?php

/**
 * @file
 * Builds placeholder replacement tokens for webform_workflow-related data.
 */

/**
 * Implements hook_token_info().
 */
function webform_workflow_token_info() {
  $info['tokens']['submission']['state'] = array(
    'name' => t('Workflow state'),
    'description' => t('The current workflow state for this submission.'),
    'type' => 'webform_workflow_state',
  );
  $info['tokens']['submission']['previous-state'] = array(
    'name' => t('Previous workflow state'),
    'description' => t('The previous workflow state for this submission.'),
    'type' => 'webform_workflow_state',
  );
  return $info;
}

/**
 * Implements hook_tokens().
 */
function webform_workflow_tokens($type, $tokens, array $data = array(), array $options = array()) {
  $replacements = array();
  if ($type == 'submission' && !empty($data['webform-submission'])) {
    $submission = $data['webform-submission'];
    if (isset($submission->webform_workflow_state)) {
      $state = $submission->webform_workflow_state;
      $sanitize = !empty($options['sanitize']);
      foreach ($tokens as $name => $original) {
        switch ($name) {
          case 'state':
            $data = $state->label;
            $replacements[$original] = $sanitize ? check_plain($data) : $data;
            break;

          case 'previous-state':
            if (isset($submission->webform_workflow_state_previous)) {
              $data = $submission->webform_workflow_state_previous->label;
              $replacements[$original] = $sanitize ? check_plain($data) : $data;
            }
            break;

        }
      }

      $current_state_tokens = token_find_with_prefix($tokens, 'state');
      if ($current_state_tokens) {
        $replacements += token_generate('webform_workflow_state', $current_state_tokens, array(
          'webform_workflow_state' => $state,
        ));
      }
      if (isset($submission->webform_workflow_state_previous) && ($previous_state_tokens = token_find_with_prefix($tokens, 'previous-state'))) {
        $replacements += token_generate('webform_workflow_state', $previous_state_tokens, array(
          'webform_workflow_state' => $submission->webform_workflow_state_previous,
        ));
      }
    }
  }
  return $replacements;
}
