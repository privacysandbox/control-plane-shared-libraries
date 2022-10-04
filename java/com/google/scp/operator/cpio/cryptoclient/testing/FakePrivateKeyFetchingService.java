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

package com.google.scp.operator.cpio.cryptoclient.testing;

import com.google.scp.operator.cpio.cryptoclient.PrivateKeyFetchingService;

/** Fake PrivateKeyFetchingService which returns a preconfigured response. */
public final class FakePrivateKeyFetchingService implements PrivateKeyFetchingService {

  private String response = "";

  /**
   * Returns the response set by the {@code setResponse} method, or an empty String if it was not
   * set.
   */
  public String fetchKeyCiphertext(String keyId) {
    return response;
  }

  /** Set a response String to be returned by the {@code fetchKeyCiphertext} method. */
  public void setResponse(String newResponse) {
    response = newResponse;
  }
}
