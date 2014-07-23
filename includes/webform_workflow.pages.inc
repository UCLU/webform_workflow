<?php
/**
 * @file
 * Page callbacks for the Webform Workflow module.
 */

/**
 * Page callback to display a list of webform submissions.
 *
 * This replaces the page at 'node/%webform_menu/webform-results', when workflow
 * is enabled for a webform. It allows the workflow state to be displayed.
 */
function webform_workflow_submissions_list($node) {
  if (!webform_workflow_is_enabled($node)) {
    module_load_include('inc', 'webform', 'includes/webform.report');
    return webform_results_submissions($node, FALSE, 50);
  }
  $view = views_get_view('webform_workflow_submissions');
  $view->override_path = current_path();
  return $view->preview('embed_1', array($node->nid));
}
