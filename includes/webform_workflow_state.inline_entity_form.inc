<?php

/**
 * @file
 * Defines the inline entity form controller for webform workflow states.
 */

class WebformWorkflowStateInlineEntityFormController extends EntityInlineEntityFormController {

  /**
  * Overrides EntityInlineEntityFormController::labels().
   */
  public function labels() {
    $labels = array(
      'singular' => t('state'),
      'plural' => t('states'),
    );
    return $labels;
  }

  /**
   * Overrides EntityInlineEntityFormController::entityForm().
   */
  public function entityForm($entity_form, &$form_state) {
    $entity = $entity_form['#entity'];
    if (!isset($entity->nid)) {
      $entity->nid = $form_state['node']->nid;
    }

    form_load_include($form_state, 'inc', 'webform_workflow', 'includes/webform_workflow_state.forms');
    webform_workflow_state_form_attach($entity_form, $form_state, $entity);

    return $entity_form;
  }

  /**
   * Overrides EntityInlineEntityFormController::tableFields().
   */
  public function tableFields($bundles) {
    $fields = array();
    $fields['label'] = array(
      'type' => 'property',
      'label' => t('Name'),
      'weight' => 1,
    );
    $fields['color'] = array(
      'type' => 'property',
      'label' => t('Color'),
      'weight' => 2,
      // For the formatter callback to work, this patch to Inline Entity Form is
      // required: https://www.drupal.org/node/2097583#comment-8997683
      'formatter callback' => array($this, 'formatColor'),
    );
    return $fields;
  }

  /**
   * Formatter callback for a state color.
   *
   * @param string $color
   *   The color (as a machine name).
   *
   * @return string
   *   An HTML string.
   */
  public function formatColor($color) {
    $color_labels = webform_workflow_state_color_options_list();
    return theme('webform_workflow_state_color', array(
      'content' => $color_labels[$color],
      'color' => $color,
    ));
  }

}
