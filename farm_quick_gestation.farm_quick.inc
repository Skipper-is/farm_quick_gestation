<?php

function farm_quick_gestation_farm_quick_forms(){
    return array(
        'farm_quick_gestation' => array(
            'label' => t('Gestation Calculator/Serving Record'),
            'permission' => 'create farm_observation log entities',
            'form' => 'farm_quick_gestation_form',
            'file' => 'farm_quick_gestation.farm_quick.inc',
        ),
    );
    
}


function farm_quick_gestation_form($form, &$form_state){

    
    $animal_types = array(
        'cow' => t('Cow (9 months/283d)'),
        'goat' => t('Goat/Sheep (5 months/150d)'),
        'pig' => t('Pig (3 months, 3 weeks, 3 days)'),
        'chicken' => t('Chicken (21 days)'),
    );

    $form['farm_quick_gestation'] = array(
        '#type' => 'fieldset',
        '#title' => t('Quick Gestation Calculator'),
        '#description' => t('This form will calculate the gestation period for a given animal type and create an observation log at that time'),
    );

    $form['farm_quick_gestation']['title'] = array(
        '#type' => 'textfield',
        '#title' => t('Title'),
        '#description' => t('Title of the log (eg: "Martha due to kid")'),
        '#required' => TRUE,
    );

    $form['farm_quick_gestation']['farm_quick_gestation_date'] = array(
        '#type' => 'date_popup',
        '#title' => t('Date of breeding/serving'),
        '#description' => t('The date at which the animal was served, or the eggs were incubated (it will calculate the due date in the next step)'),
        '#date_format' => 'Y-m-d',
        '#date_year_range' => '-5:+5',
        '#date_label_position' => 'within',
        '#default_value' => date('Y-m-d'),
    );

    $form['farm_quick_gestation']['farm_quick_gestation_notes'] = array(
        '#type' => 'textarea',
        '#title' => t('Notes'),
        '#description' => t('Any notes about the breeding/serving'),
        '#default_value' => '',
    );

    $form['farm_quick_gestation']['asset'] = array(
        '#type'=>'textfield',
        '#title' => t('Group/animals'),
        '#description' => t('Animals involved (could be sire and dam, or just dam) - Separate with comma'),
        '#autocomplete_path' => 'farm_asset/autocomplete/animal+group',
          );


    $form['farm_quick_gestation']['species'] = array(
        '#type' => 'radios',
        '#title' => t('Species'),
        '#description' => t('Select the species of the animal'),
        '#options' => $animal_types,
        '#default_value' => 'goat',
        '#required' => TRUE,
    );

    $form['farm_quick_gestation']['farm_quick_gestation_submit'] = array(
        '#type' => 'submit',
        '#value' => t('Submit'),
    );
    return $form;
}

function farm_quick_gestation_form_validate($form, &$form_state){
    $assets = array();
    $asset_names = drupal_explode_tags($form_state['values']['asset']);
    foreach($asset_names as $asset_name){
      $id = 0;
      $matches = array();
      $result =preg_match('/\\[id: ([0-9]+)\\]/', $asset_name, $matches);
      if(!empty($matches[$result])){
        $id = $matches[$result];
      }
        if(empty($id)){
            form_set_error('farm_quick_gestation] [asset', t('Could not load the animal, Make sure the asset ID is included, for example "Peter"[id:1234]"'));
           }
        $asset = farm_asset_load($id);
              if (empty($asset)){
                form_set_error('farm_quick_gestation] [asset', t('Could not load the animal, Make sure the ID and name are correct'));
              }
              if (!empty($asset)){
                $assets[$id] = $asset;
              }
    }

    $form_state['storage']['assets'] = $assets;

}

function farm_quick_gestation_form_submit($form, &$form_state){
    $log_type = "farm_observation";
    $log_name = $form_state['values']['title'];
    $timestamp = strtotime($form_state['values']['farm_quick_gestation_date']);
    $due_date = calculate_due_date($form_state['values']['species'], $timestamp);
    $date_created = t("Log created on " . date('Y-m-d', $timestamp));
    $notes = $form_state['values']['farm_quick_gestation_notes'] . "\r\n" . $date_created;
    $assets = $form_state['storage']['assets'];
    $asset=reset($assets);
    $done = FALSE;

    $log = farm_log_create($log_type, $log_name, $due_date, $done, $assets, $notes, $categories = array('Animals'));
    $log_wrapper = entity_metadata_wrapper('log', $log);
    $log_wrapper->save();

}

function calculate_due_date($animal,$timestamp){
    
    $animal_gestation = array(
        'cow' => array(
            'name' => 'Cow',
            'gestation' => 283,
        ),
        'goat' => array(
            'name' => 'Sheep/goat',
            'gestation' => 147,
        ),
        'pig' => array(
            'name' => 'Pig',
            'gestation' => 114,
        ),
        'chicken' => array(
            'name' => 'Chicken',
            'gestation' => 21,
        ),
    );

    $due_date = $timestamp + ($animal_gestation[$animal]['gestation'] * 86400);
    return $due_date;
}