<?php

/**
 * @file
 * Default theme implementation for the petitionadmin dashboard daily stats.
 *
 * This template needs to be run through an inline CSS tool such as
 * http://templates.mailchimp.com/resources/inline-css/
 *
 * Available variables are from PetitionsStatistics::getPetitionStatistics()
 */

?>
<head>
  <style type="text/css">
    body {
      font-family: Arial, Helvetica, sans-serif;
      font-size: 14px;
    }

    .petitionadmin_dashboard h2 {
      font-size:1.2em;
    }

    .petitionadmin_dashboard .section_header {
      margin-top: 20px;
      font-size: 1.2em;
      font-weight: bold;
    }
    .petitionadmin_dashboard .header .report_date {
      float: right;
      font-size: 0.8em;
      line-height: 20px;
    }

    .petitionadmin_dashboard .header .report_time_period {
      float: left;
      font-size: 1.2em;
      font-weight: bold;
      font-style: italic;
    }

    .petitionadmin_dashboard .dashboard_statistics {
      margin-left: 20px;
    }
    .petitionadmin_dashboard .dashboard_statistics .statistics_row .col_label {
      float: left;
      width: 300px;
    }

    .clearfix {
      clear: both;
    }

    .petitionadmin_dashboard .alert_description .col_label,
    .petitionadmin_dashboard .petition_alert .col_label {
      font-weight: bold;
      float: left;
      padding-right: 20px;
    }

    .petitionadmin_dashboard .petition_alert {
      margin-left: 20px;
    }

    .petitionadmin_dashboard .top_signed_petitions .signature_count {
      width: 70px;
      float: left;
      margin-right: 10px;
      text-align: right;
    }

    .petitionadmin_dashboard .fraud_alert {
      margin-bottom: 10px;
    }
  </style>
</head>
<body>

<img src="https://petitions.whitehouse.gov/profiles/petitions/modules/custom/petitions_homepage/img/petitions_landing_hero.jpg" width=300>

<div class="petitionadmin_dashboard">
  <h1>Petitions.whitehouse.gov Daily Statistics</h1>
  <div class="header">
    <div class="report_time_period">Last <?php print $time_period?> Hours Report</div>
    <div class="report_date"><strong>Report Date:</strong> <?php print date('Y-m-d h:i A');?></div>
  </div>
  <div class="clearfix"></div>
  <hr>
  <div class="dashboard_statistics">
    <div class="statistics_row">
      <div class="col_label">Petitions Created:</div>
      <div class="col_value"><?php print $petitions_created ?></div>
    </div>
    <div class="statistics_row">
      <div class="col_label">Petitions Published:</div>
      <div class="col_value"><?php print $petitions_created ?></div>
    </div>
    <div class="statistics_row">
      <div class="col_label">Petitions Reached Public:</div>
      <div class="col_value"><?php print $petitions_reached_public ?></div>
    </div>
    <div class="statistics_row">
      <div class="col_label">Petitions Reached Ready For Response:</div>
      <div class="col_value"><?php print $petitions_reached_ready_for_response ?></div>
    </div>
    <div class="statistics_row">
      <div class="col_label">Signatures Started:</div>
      <div class="col_value"><?php print $signatures_started ?></div>
    </div>
    <div class="statistics_row">
      <div class="col_label">Signatures Verified:</div>
      <div class="col_value"><?php print $signatures_verified ?></div>
    </div>
    <div class="statistics_row">
      <div class="col_label"># of Petitions Crossing <?php print $signature_threshold_1 ?> Signatures:</div>
      <div class="col_value"><?php print $crossed_signature_threshold_1 ?></div>
    </div>
    <div class="statistics_row">
      <div class="col_label"># of Petitions Crossing <?php print $signature_threshold_2 ?> Signatures:</div>
      <div class="col_value"><?php print $crossed_signature_threshold_2 ?></div>
    </div>
    <div class="statistics_row">
      <div class="col_label"># of Petitions Crossing <?php print $signature_threshold_3 ?> Signatures:</div>
      <div class="col_value"><?php print $crossed_signature_threshold_3 ?></div>
    </div>
    <div class="statistics_row">
      <div class="col_label">Avg time from Signature Initiated to<br>Validation sent:</div>
      <div class="col_value"><?php print $signature_to_initiated_validation_elapsed ?> ms</div>
    </div>
  </div>
  <div class="clearfix"></div>
  <div class="section_header">Top 5 Petitions</div>
  <hr>
  <?php foreach ($top_signed_petitions as $petition_id => $data) :?>
    <div class="top_signed_petitions">
      <div class="signature_count"><?php print number_format($data->signature_count) ?></div>
      <div class="petition_link">
        <a href="https://petitions.whitehouse.gov/node/<?php print $petition_id ?>" title="<?php print $petition_id ?>" target="_blank"><?php print $data->title ?></a>
      </div>
    </div>
    <div class="clearfix"></div>
  <?php endforeach;?>

  <div class="section_header">Fraud Alerts</div>
  <hr>
  <?php foreach ($alerts as $alert_description => $alert_data) :?>
    <div class="fraud_alert">
      <div class="alert_description">
        <div class="col_label">Alert:</div>
        <div class="col_value"><?php print $alert_description ?></div>
      </div>
      <?php foreach ($alert_data as $petition_id => $data) :?>
        <div class="petition_alert">
          <div class="petition_link">
            <a href="https://petitions.whitehouse.gov/node/<?php print $petition_id ?>" title="<?php print $petition_id ?>" target="_blank"><?php print $data['petition_title'] ?></a>
          </div>
        </div>
        <div class="clearfix"></div>
      <?php endforeach;?>
    </div>
  <?php endforeach; ?>
</div>
</body>
