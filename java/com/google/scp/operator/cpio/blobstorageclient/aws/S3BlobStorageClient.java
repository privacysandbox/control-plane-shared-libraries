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

package com.google.scp.operator.cpio.blobstorageclient.aws;

import com.google.common.collect.ImmutableList;
import com.google.inject.Inject;
import com.google.scp.operator.cpio.blobstorageclient.BlobStorageClient;
import com.google.scp.operator.cpio.blobstorageclient.model.DataLocation;
import com.google.scp.operator.cpio.blobstorageclient.model.DataLocation.BlobStoreDataLocation;
import java.io.InputStream;
import java.nio.file.Path;
import software.amazon.awssdk.core.exception.SdkException;
import software.amazon.awssdk.services.s3.S3Client;
import software.amazon.awssdk.services.s3.model.DeleteObjectRequest;
import software.amazon.awssdk.services.s3.model.GetObjectRequest;
import software.amazon.awssdk.services.s3.model.ListObjectsV2Request;
import software.amazon.awssdk.services.s3.model.PutObjectRequest;
import software.amazon.awssdk.services.s3.model.S3Object;
import software.amazon.awssdk.services.s3.paginators.ListObjectsV2Iterable;

/**
 * S3BlobStorageClient implements the {@code BlobStorageClient} interface for AWS S3 storage
 * service.
 */
public final class S3BlobStorageClient implements BlobStorageClient {

  private S3Client client;

  /** Creates a new instance of {@code S3BlobStorageClient}. */
  @Inject
  public S3BlobStorageClient(S3Client client) {
    this.client = client;
  }

  @Override
  public InputStream getBlob(DataLocation location) throws BlobStorageClientException {
    BlobStoreDataLocation blobLocation = location.blobStoreDataLocation();
    try {
      return client.getObject(
          GetObjectRequest.builder().bucket(blobLocation.bucket()).key(blobLocation.key()).build());
    } catch (SdkException exception) {
      throw new BlobStorageClientException(exception);
    }
  }

  @Override
  public void putBlob(DataLocation location, Path filePath) throws BlobStorageClientException {
    BlobStoreDataLocation blobLocation = location.blobStoreDataLocation();
    try {
      client.putObject(
          PutObjectRequest.builder().bucket(blobLocation.bucket()).key(blobLocation.key()).build(),
          filePath);
    } catch (SdkException exception) {
      throw new BlobStorageClientException(exception);
    }
  }

  @Override
  public void deleteBlob(DataLocation location) throws BlobStorageClientException {
    BlobStoreDataLocation blobLocation = location.blobStoreDataLocation();
    try {
      client.deleteObject(
          DeleteObjectRequest.builder()
              .bucket(blobLocation.bucket())
              .key(blobLocation.key())
              .build());
    } catch (SdkException exception) {
      throw new BlobStorageClientException(exception);
    }
  }

  @Override
  public ImmutableList<String> listBlobs(DataLocation location) throws BlobStorageClientException {
    BlobStoreDataLocation blobLocation = location.blobStoreDataLocation();
    try {
      ListObjectsV2Iterable objectsV2Iterable =
          client.listObjectsV2Paginator(
              ListObjectsV2Request.builder()
                  .bucket(blobLocation.bucket())
                  .prefix(blobLocation.key())
                  .build());
      return objectsV2Iterable.contents().stream()
          .map(S3Object::key)
          .collect(ImmutableList.toImmutableList());
    } catch (SdkException exception) {
      throw new BlobStorageClientException(exception);
    }
  }
}
