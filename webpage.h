#ifndef WEBPAGE_H
#define WEBPAGE_H

#include <Arduino.h>

const char htmlPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 Image Manager</title>
    <style>
        * { margin:0; padding:0; box-sizing:border-box; }
        body { font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
               background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); min-height:100vh; padding:20px; }
        .container { max-width:1200px; margin:0 auto; background:white; border-radius:20px; padding:30px;
                     box-shadow:0 20px 60px rgba(0,0,0,0.3); }
        h1 { color:#333; margin-bottom:10px; text-align:center; }
        .subtitle { text-align:center; color:#666; margin-bottom:30px; }
        .status.connected { background:#e8f5e9; color:#2e7d32; padding:15px; border-radius:10px;
                             margin-bottom:20px; text-align:center; font-weight:600; }
        .upload-section { background:#f5f5f5; padding:20px; border-radius:10px; margin-bottom:30px; }
        input[type="file"] { width:100%; padding:12px; border:2px dashed #667eea; border-radius:8px; cursor:pointer; }
        button { padding:12px 24px; border:none; border-radius:8px; font-size:16px; font-weight:600;
                 cursor:pointer; transition:all 0.3s; }
        .btn-primary { background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); color:white; }
        .btn-primary:hover { transform: translateY(-2px); box-shadow:0 5px 15px rgba(0,0,0,0.2); }
        .btn-danger { background:#f44336; color:white; padding:8px 16px; font-size:14px; }
        .btn-view { background:#2196f3; color:white; padding:8px 16px; font-size:14px; margin-right:10px; }
        .progress-bar { width:100%; height:30px; background:#e0e0e0; border-radius:15px; overflow:hidden; margin-top:15px; display:none; }
        .progress-fill { height:100%; background: linear-gradient(90deg, #667eea 0%, #764ba2 100%); width:0%;
                         transition: width 0.3s; display:flex; align-items:center; justify-content:center; color:white;
                         font-weight:600; }
        .upload-stats { margin-top:15px; padding:10px; background:#e3f2fd; border-radius:8px; display:none; }
        .upload-stats.show { display:block; }
        .connection-stats { margin-top:10px; padding:10px; background:#fff3e0; border-radius:8px; border-left:4px solid #ff9800; }
        .stat-row { display:flex; justify-content:space-between; padding:5px 0; font-size:14px; }
        .stat-label { color:#666; }
        .stat-value { font-weight:600; color:#1976d2; }
        .connection-value { font-weight:600; color:#f57c00; }
        .gallery { display:grid; grid-template-columns:repeat(auto-fill,minmax(250px,1fr)); gap:20px; margin-top:20px; }
        .image-card { background:#f5f5f5; border-radius:10px; overflow:hidden; box-shadow:0 2px 8px rgba(0,0,0,0.1); }
        .image-card img { width:100%; height:200px; object-fit:cover; cursor:pointer; }
        .image-info { padding:15px; }
        .image-name { font-weight:600; margin-bottom:5px; word-break:break-all; }
        .image-size { color:#666; font-size:14px; margin-bottom:10px; }
        .image-actions { display:flex; gap:10px; }
        .empty-state { text-align:center; padding:60px 20px; color:#999; }
        .instructions { background:#fff3e0; padding:20px; border-radius:10px; margin-bottom:30px; line-height:1.5; }
        .modal { display:none; position:fixed; z-index:1000; left:0; top:0; width:100%; height:100%; background:rgba(0,0,0,0.9);
                 justify-content:center; align-items:center; }
        .modal.active { display:flex; }
        .modal img { max-width:90%; max-height:90%; border-radius:10px; }
        .modal-close { position:absolute; top:20px; right:40px; color:white; font-size:40px; cursor:pointer; font-weight:300; }
        @media (max-width:768px) { .gallery { grid-template-columns:repeat(auto-fill,minmax(150px,1fr)); } }
    </style>
</head>
<body>
<div class="container">
<h1>📷 ESP32 Image Manager</h1>
<p class="subtitle">Upload, view, and manage images on your ESP32</p>

<div class="status connected">Connected to ESP32</div>

<div class="upload-section">
<h3>Upload Image</h3>
<input type="file" id="fileInput" accept="image/*" multiple>
<button class="btn-primary" onclick="uploadFiles()">Upload Selected Images</button>
<div class="progress-bar" id="progressBar"><div class="progress-fill" id="progressFill">0%</div></div>
<div class="upload-stats" id="uploadStats">
<div class="stat-row"><span class="stat-label">Total Upload Time:</span><span class="stat-value" id="totalTime">-</span></div>
<div class="stat-row"><span class="stat-label">Average Time per Image:</span><span class="stat-value" id="avgTime">-</span></div>
<div class="stat-row"><span class="stat-label">Upload Speed:</span><span class="stat-value" id="uploadSpeed">-</span></div>
<div class="stat-row"><span class="stat-label">Total Size:</span><span class="stat-value" id="totalSize">-</span></div>
</div>
</div>

<div class="instructions">
<h3>How-To: Create a New Image for Display</h3>
<ol>
<li style="padding: 5px;">Go to an online image editor (e.g., <a href="https://www.online-image-editor.com/" target="_blank">online-image-editor.com</a>).</li>
<li style="padding: 5px;">Upload any image you wish. This should open a new editing window.</li>
<li style="padding: 5px;">In the new window, select <strong>Resize</strong>.</li>
<li style="padding: 5px;">Uncheck <strong>Aspect Ratio</strong> and set Width = 256, Height = 31.</li>
<li style="padding: 5px;">Click <strong>Apply</strong>.</li>
<li style="padding: 5px;">Select <strong>Rotate/Flip</strong> and rotate <strong>+90°</strong>.</li>
<li style="padding: 5px;">Go to the <strong>Advanced</strong> tab and select <strong>Convert</strong>. Choose BMP format.</li>
<li style="padding: 5px;">Click <strong>Apply</strong>, then <strong>Save</strong> and download the BMP image to your computer.</li>
<li style="padding: 5px;">Upload the saved BMP image using the upload section above.</li>
<li style="padding: 5px;">Wait for it to appear on the globe display!</li>
</ol>
</div>

<div class="connection-stats">
<div class="stat-row"><span class="stat-label">⏱️ Connection Uptime:</span><span class="connection-value" id="connectionTime">Calculating...</span></div>
</div>

<h3>Image Gallery</h3>
<div class="gallery" id="gallery"><div class="empty-state"><p>No images yet. Upload some images to get started!</p></div></div>

</div>

<div class="modal" id="imageModal" onclick="closeModal()"><span class="modal-close">&times;</span><img id="modalImage" src="" alt="Full size image"></div>

<script>
        // Track connection start time
        const connectionStartTime = performance.now();
        
        // Update connection time every second
        function updateConnectionTime() {
            const now = performance.now();
            const elapsed = (now - connectionStartTime) / 1000;
            
            const hours = Math.floor(elapsed / 3600);
            const minutes = Math.floor((elapsed % 3600) / 60);
            const seconds = Math.floor(elapsed % 60);
            
            let timeString = '';
            if (hours > 0) {
                timeString = `${hours}h ${minutes}m ${seconds}s`;
            } else if (minutes > 0) {
                timeString = `${minutes}m ${seconds}s`;
            } else {
                timeString = `${seconds}s`;
            }
            
            document.getElementById('connectionTime').textContent = timeString;
        }
        
        // Update every second
        setInterval(updateConnectionTime, 1000);
        updateConnectionTime(); // Initial update
        
        // Load images on page load
        window.onload = () => {
            loadImages();
        };
        
        async function loadImages() {
            try {
                const response = await fetch('/list');
                const images = await response.json();
                
                const gallery = document.getElementById('gallery');
                gallery.innerHTML = '';
                
                if (images.length === 0) {
                    gallery.innerHTML = `
                        <div class="empty-state">
                            <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" stroke="currentColor">
                                <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M4 16l4.586-4.586a2 2 0 012.828 0L16 16m-2-2l1.586-1.586a2 2 0 012.828 0L20 14m-6-6h.01M6 20h12a2 2 0 002-2V6a2 2 0 00-2-2H6a2 2 0 00-2 2v12a2 2 0 002 2z" />
                            </svg>
                            <p>No images yet. Upload some images to get started!</p>
                        </div>
                    `;
                    return;
                }
                
                images.forEach(image => {
                    const card = document.createElement('div');
                    card.className = 'image-card';
                    card.innerHTML = `
                        <img src="/image?file=${encodeURIComponent(image.name)}" alt="${image.name}" onclick="viewImage('${image.name}')">
                        <div class="image-info">
                            <div class="image-name">${image.name}</div>
                            <div class="image-size">${formatBytes(image.size)}</div>
                            <div class="image-actions">
                                <button class="btn-view" onclick="viewImage('${image.name}')">View</button>
                                <button class="btn-danger" onclick="deleteImage('${image.name}')">Delete</button>
                            </div>
                        </div>
                    `;
                    gallery.appendChild(card);
                });
            } catch (error) {
                console.error('Error loading images:', error);
                alert('Failed to load images');
            }
        }
        
        async function uploadFiles() {
            const fileInput = document.getElementById('fileInput');
            const files = fileInput.files;
            
            if (files.length === 0) {
                alert('Please select at least one image');
                return;
            }
            
            const progressBar = document.getElementById('progressBar');
            const progressFill = document.getElementById('progressFill');
            const uploadStats = document.getElementById('uploadStats');
            
            progressBar.style.display = 'block';
            uploadStats.classList.remove('show');
            
            const startTime = performance.now();
            let totalBytes = 0;
            
            // Calculate total size
            for (let i = 0; i < files.length; i++) {
                totalBytes += files[i].size;
            }
            
            for (let i = 0; i < files.length; i++) {
                const file = files[i];
                const formData = new FormData();
                formData.append('file', file);
                
                try {
                    progressFill.textContent = `Uploading ${i + 1} of ${files.length}...`;
                    progressFill.style.width = ((i / files.length) * 100) + '%';
                    
                    const fileStartTime = performance.now();
                    const response = await fetch('/upload', {
                        method: 'POST',
                        body: formData
                    });
                    const fileEndTime = performance.now();
                    const fileUploadTime = ((fileEndTime - fileStartTime) / 1000).toFixed(2);
                    
                    console.log(`${file.name}: ${fileUploadTime}s (${formatBytes(file.size)})`);
                    
                    if (!response.ok) {
                        throw new Error('Upload failed');
                    }
                } catch (error) {
                    console.error('Upload error:', error);
                    alert(`Failed to upload ${file.name}`);
                }
            }
            
            const endTime = performance.now();
            const totalTime = ((endTime - startTime) / 1000).toFixed(2);
            const avgTime = (totalTime / files.length).toFixed(2);
            const uploadSpeed = (totalBytes / 1024 / (totalTime)).toFixed(2);
            
            progressFill.textContent = 'Complete!';
            progressFill.style.width = '100%';
            
            // Display statistics
            document.getElementById('totalTime').textContent = `${totalTime}s`;
            document.getElementById('avgTime').textContent = `${avgTime}s`;
            document.getElementById('uploadSpeed').textContent = `${uploadSpeed} KB/s`;
            document.getElementById('totalSize').textContent = formatBytes(totalBytes);
            uploadStats.classList.add('show');
            
            console.log(`\n=== Upload Complete ===`);
            console.log(`Total time: ${totalTime}s`);
            console.log(`Average time: ${avgTime}s per image`);
            console.log(`Upload speed: ${uploadSpeed} KB/s`);
            console.log(`Total size: ${formatBytes(totalBytes)}`);
            
            setTimeout(() => {
                progressBar.style.display = 'none';
                progressFill.style.width = '0%';
                fileInput.value = '';
                loadImages();
            }, 1500);
        }
        
        async function deleteImage(filename) {
            if (!confirm(`Delete ${filename}?`)) {
                return;
            }
            
            try {
                const response = await fetch('/delete', {
                    method: 'POST',
                    headers: {
                        'Content-Type': 'application/x-www-form-urlencoded',
                    },
                    body: 'filename=' + encodeURIComponent(filename)
                });
                
                if (response.ok) {
                    loadImages();
                } else {
                    alert('Failed to delete image');
                }
            } catch (error) {
                console.error('Delete error:', error);
                alert('Failed to delete image');
            }
        }
        
        function viewImage(filename) {
            const modal = document.getElementById('imageModal');
            const modalImage = document.getElementById('modalImage');
            modalImage.src = '/image?file=' + encodeURIComponent(filename);
            modal.classList.add('active');
        }
        
        function closeModal() {
            const modal = document.getElementById('imageModal');
            modal.classList.remove('active');
        }
        
        function formatBytes(bytes) {
            if (bytes === 0) return '0 Bytes';
            const k = 1024;
            const sizes = ['Bytes', 'KB', 'MB'];
            const i = Math.floor(Math.log(bytes) / Math.log(k));
            return Math.round(bytes / Math.pow(k, i) * 100) / 100 + ' ' + sizes[i];
        }
    </script>
</body>
</html>
)rawliteral";

#endif
