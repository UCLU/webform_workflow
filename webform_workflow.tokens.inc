<?php

/**
 * @file
 * Builds placeholder replacement tokens for webform_workflow-related data.
 */

/**
 * Implements hook_token_info().
 */
function webform_workflow_token_info() {
  // Webform workflow submission tokens.
  $info['types']['submission-workflow'] = array(
    'name' => t('Submission'),
    'description' => t('Tokens related to webform submission workflow state.'),
    'needs-data' => 'webform-workflow-submission',
  );

  $info['tokens']['submission-workflow']['sid'] = array(
    'name' => t('Submission ID'),
    'description' => t('The unique identifier for the webform submission.'),
  );
  $info['tokens']['submission-workflow']['nid'] = array(
    'name' => t('Webform node ID'),
    'description' => t('The unique identifier for the webform.'),
    'type' => 'node',
  );
  $info['tokens']['submission-workflow']['wsid'] = array(
    'name' => t('Workflow state'),
    'description' => t('The current workflow state of the submission.'),
  );

  return $info;
}

/**
 * Implements hook_tokens().
 */
function webform_workflow_tokens($type, $tokens, array $data = array(), array $options = array()) {
  $replacements = array();

  // Webform tokens (caching globally).
  if ($type == 'submission-workflow' && !empty($data['webform-submission-workflow'])) {
    // Prepare all the submission data that we will likely need more than once.
    $submission = $data['webform-submission-workflow'];
    $node = isset($data['node']) ? $data['node'] : node_load($submission->nid);
    $email = isset($data['webform-email']) ? $data['webform-email'] : NULL;

    // Replace individual tokens that have an exact replacement.
    foreach ($tokens as $name => $original) {
      switch ($name) {
        case 'sid':
          $replacements[$original] = $submission->sid;
          break;
      }
    }

    // Chained token relationships.
    if ($date_tokens = token_find_with_prefix($tokens, 'date')) {
      $replacements += token_generate('date', $date_tokens, array('date' => $submission->submitted), $options);
    }
    if (($user_tokens = token_find_with_prefix($tokens, 'user')) && $account = user_load($submission->uid)) {
      $replacements += token_generate('user', $user_tokens, array('user' => $account), $options);
    }
  }

  return $replacements;
}
