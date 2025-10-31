// Main JavaScript

document.addEventListener('DOMContentLoaded', function () {
	const statusElement = document.getElementById('status');

	// Update status message
	statusElement.textContent = '✅ All external resources loaded successfully!';
	statusElement.className = 'loaded-message';

	// Log to console
	console.log('✅ JavaScript loaded from /assets/scripts/main.js');
	console.log('✅ CSS loaded from /assets/styles/main.css');
	console.log('✅ Image loaded from /assets/images/sample.jpg');

	// Add click handler to image
	const img = document.querySelector('main img');
	if (img) {
		img.addEventListener('click', function () {
			alert('Image clicked! This was loaded from an external location.');
		});
		img.style.cursor = 'pointer';
	}
});

// Log when script starts executing
console.log('🚀 External JavaScript file is running!');
