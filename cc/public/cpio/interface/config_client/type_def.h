/*
 * Copyright 2022 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SCP_CPIO_INTERFACE_CONFIG_CLIENT_TYPE_DEF_H_
#define SCP_CPIO_INTERFACE_CONFIG_CLIENT_TYPE_DEF_H_

#include <string>
#include <vector>

namespace google::scp::cpio {
using TagName = std::string;
using TagValue = std::string;
using ParameterName = std::string;
using ParameterValue = std::string;
using InstanceId = std::string;

/// Configurations for ConfigClient.
struct ConfigClientOptions {
  virtual ~ConfigClientOptions() = default;

  /**
   * @brief The tag values for this given tag_names should be
   * written to cloud first. In AWS, it should be a label in EC2 instance, and
   * in GCP, it is stored in Metadata Service. CPIO will fetch the
   * them during initialization time, store it in memory and serve
   * them through ConfigClient.
   */
  std::vector<TagName> tag_names;
  /**
   * @brief The parameter values for this given parameter_names should be
   * written to cloud first. In AWS, they should be stored in the Parameter
   * Store, and in GCP, they should be stored in the Secret Manager. CPIO will
   * fetch these parameters during initialization time, store them in memory and
   * serve them through ConfigClient.
   */
  std::vector<ParameterName> parameter_names;
};
}  // namespace google::scp::cpio

#endif  // SCP_CPIO_INTERFACE_CONFIG_CLIENT_TYPE_DEF_H_
