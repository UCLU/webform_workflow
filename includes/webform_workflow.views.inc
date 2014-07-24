<?php
/**
 * @file
 * Views integration for the Webform Workflow module.
 */

/**
 * Implements hook_views_data().
 */
function webform_workflow_views_data() {
  $data = array();

  $data['webform_workflow_log']['table'] = array(
    'group' => t('Webform Workflow Log'),
    'base' => array(
      'title' => t('Webform Workflow Log'),
      'field' => 'id',
      'help' => t('Contains information about webform submission state changes.'),
    ),
    'join' => array(
      'node' => array(
        'left_field' => 'nid',
        'field' => 'nid',
      ),
    ),
  );

  $data['webform_workflow_log']['id'] = array(
    'title' => t('ID'),
    'help' => t('The primary key for this log message.'),
    'sort' => array(
      'views_handler_sort_numeric',
    ),
  );

  $data['webform_workflow_log']['nid'] = array(
    'title' => t('Node'),
    'help' => t('The webform node this submission was generated from.'),
    'relationship' => array(
      'base' => 'node',
      'field' => 'nid',
      'handler' => 'views_handler_relationship',
      'label' => t('Webform Node'),
    ),
  );

  $data['webform_workflow_log']['sid'] = array(
    'title' => t('Submission'),
    'help' => t('The webform submission undergoing the state change.'),
    'relationship' => array(
      'base' => 'webform_submissions',
      'field' => 'sid',
      'handler' => 'views_handler_relationship',
      'label' => t('Webform Submission'),
    ),
  );

  $data['webform_workflow_log']['uid'] = array(
    'title' => t('User'),
    'help' => t('The user who made the state change.'),
    'relationship' => array(
      'base' => 'users',
      'field' => 'uid',
      'handler' => 'views_handler_relationship',
      'label' => t('Webform Workflow User'),
    ),
  );

  $data['webform_workflow_log']['timestamp'] = array(
    'title' => t('Date changed'),
    'help' => t('The date and time of the state change.'),
    'field' => array(
      'handler' => 'views_handler_field_date',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_date',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_date',
    ),
  );

  $data['webform_workflow_log']['wsid_from'] = array(
    'title' => t('Previous State'),
    'help' => t('The previous state of the submission.'),
    'relationship' => array(
      'base' => 'webform_workflow_states',
      'field' => 'wsid_from',
      'left_field' => 'wsid',
      'handler' => 'views_handler_relationship',
      'label' => t('Previous Submission State'),
    ),
  );

  $data['webform_workflow_log']['wsid_to'] = array(
    'title' => t('New State'),
    'help' => t('The new state of the submission.'),
    'relationship' => array(
      'base' => 'webform_workflow_states',
      'field' => 'wsid_to',
      'left_field' => 'wsid',
      'handler' => 'views_handler_relationship',
      'label' => t('New Submission State'),
    ),
  );

  $data['webform_workflow_log']['message'] = array(
    'title' => t('Log message'),
    'help' => t('The log message associated with this state change.'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  return $data;
}

/**
 * Implements hook_views_data_alter().
 */
function webform_workflow_views_data_alter(&$data) {

  // Make workflow state data available for webform submissions.
  $data['webform_workflow_submissions']['table']['join'] = array(
    'webform_submissions' => array(
      'left_field' => 'sid',
      'field' => 'sid',
    ),
  );
  $data['webform_workflow_states']['table']['join'] = array(
    'webform_submissions' => array(
      'left_field' => 'wsid',
      'field' => 'wsid',
      'left_table' => 'webform_workflow_submissions',
    ),
  );

  // Provide an improved 'Draft' field handler.
  $data['webform_submissions']['is_draft_bool'] = array(
    'title' => t('Draft'),
    'help' => t('Whether or not the submission is a draft (handled as a Boolean value).'),
    'field' => array(
      'handler' => 'views_handler_field_boolean',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_boolean',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'real field' => 'is_draft',
  );

}
