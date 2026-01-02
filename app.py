"""
Trash Classifier Server - Main Application
IoT Smart Bin Project

Flask REST API để phân loại rác thông minh.
"""

from flask import Flask
from src.api.classify import classify_bp
from src.api.health import health_bp
from src.config import Config
from src.model_loader import ModelLoader
from src.utils.logger import logger

# Load configuration
config = Config('config.yaml')

# Pre-load model
logger.info("Loading YOLO model...")
model_loader = ModelLoader(config)
model = model_loader.get_model()
logger.info("Model loaded successfully!")

# Create Flask app
app = Flask(__name__)
app.config['MAX_CONTENT_LENGTH'] = config.get('server.max_content_length', 5 * 1024 * 1024)

# Register blueprints
app.register_blueprint(classify_bp)
app.register_blueprint(health_bp)

# Error handlers
@app.errorhandler(413)
def request_entity_too_large(error):
    return {
        'success': False,
        'error': 'File too large. Maximum size is 5MB.'
    }, 413

@app.errorhandler(500)
def internal_server_error(error):
    return {
        'success': False,
        'error': 'Internal server error'
    }, 500


if __name__ == '__main__':
    host = config.get('server.host', '0.0.0.0')
    port = config.get('server.port', 5000)
    debug = config.get('server.debug', False)
    
    logger.info(f"Starting server at {host}:{port}")
    app.run(host=host, port=port, debug=debug)
