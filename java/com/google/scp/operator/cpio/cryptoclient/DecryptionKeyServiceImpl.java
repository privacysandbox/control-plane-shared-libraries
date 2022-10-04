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

package com.google.scp.operator.cpio.cryptoclient;

import com.google.common.cache.CacheBuilder;
import com.google.common.cache.CacheLoader;
import com.google.common.cache.LoadingCache;
import com.google.common.util.concurrent.UncheckedExecutionException;
import com.google.crypto.tink.Aead;
import com.google.crypto.tink.HybridDecrypt;
import com.google.crypto.tink.JsonKeysetReader;
import com.google.crypto.tink.KeysetHandle;
import com.google.inject.Inject;
import com.google.scp.operator.cpio.cryptoclient.PrivateKeyFetchingService.PrivateKeyFetchingServiceException;
import com.google.scp.operator.cpio.cryptoclient.model.ErrorReason;
import java.io.IOException;
import java.security.GeneralSecurityException;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.TimeUnit;

/** Interface responsible for retrieving and decrypting keys from the aggregate service KMS */
public final class DecryptionKeyServiceImpl implements DecryptionKeyService {

  private static final int MAX_CACHE_SIZE = 100;
  private static final long CACHE_ENTRY_TTL_SEC = 3600;
  private static final int CONCURRENCY_LEVEL = Runtime.getRuntime().availableProcessors();
  // TODO: Once the key ARNs are returned from the private key vending service,
  //   we would probably want to switch to cached AEAD supplier instead
  //   of injecting an aead instance.
  private final Aead aead;
  private final PrivateKeyFetchingService privateKeyFetchingService;
  private final LoadingCache<String, HybridDecrypt> decypterCache =
      CacheBuilder.newBuilder()
          .maximumSize(MAX_CACHE_SIZE)
          .expireAfterWrite(CACHE_ENTRY_TTL_SEC, TimeUnit.SECONDS)
          .concurrencyLevel(CONCURRENCY_LEVEL)
          .build(
              new CacheLoader<String, HybridDecrypt>() {
                @Override
                public HybridDecrypt load(final String keyId) throws KeyFetchException {
                  try {
                    var keySetHandle =
                        KeysetHandle.read(
                            JsonKeysetReader.withString(
                                privateKeyFetchingService.fetchKeyCiphertext(keyId)),
                            aead);
                    return keySetHandle.getPrimitive(HybridDecrypt.class);
                  } catch (IOException
                      | GeneralSecurityException
                      | PrivateKeyFetchingServiceException e) {
                    throw new KeyFetchException(
                        "Failed to fetch key ID: " + keyId, ErrorReason.UNKNOWN_ERROR, e);
                  }
                }
              });

  /** Creates a new instance of the {@code DecryptionKeyServiceImpl} class. */
  @Inject
  public DecryptionKeyServiceImpl(Aead aead, PrivateKeyFetchingService privateKeyFetchingService) {
    this.aead = aead;
    this.privateKeyFetchingService = privateKeyFetchingService;
  }

  /** Returns the decrypter for the provided key. */
  @Override
  public HybridDecrypt getDecrypter(String keyId) throws KeyFetchException {
    try {
      return decypterCache.get(keyId);
    } catch (ExecutionException | UncheckedExecutionException e) {
      throw new KeyFetchException("Failed to get key ID: " + keyId, ErrorReason.UNKNOWN_ERROR, e);
    }
  }
}
