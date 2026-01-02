"""
Health Check API Blueprint
Endpoints để kiểm tra trạng thái server
"""

from flask import Blueprint, jsonify

from ..config import Config
from ..model_loader import ModelLoader
from ..utils import ResponseBuilder


# Create Blueprint
health_bp = Blueprint('health', __name__)


@health_bp.route('/health', methods=['GET'])
def health():
    """Health check endpoint - trả về status và model loaded."""
    try:
        config = Config()
        model_loader = ModelLoader(config)
        
        # Check model status
        model_loaded = model_loader.is_loaded()
        
        # Try to load model if not loaded
        if not model_loaded:
            try:
                model_loader.get_model()
                model_loaded = True
            except Exception:
                model_loaded = False
        
        status = "healthy" if model_loaded else "degraded"
        version = config.get('server.version', '1.0.0')
        
        response = ResponseBuilder.health_check(
            status=status,
            model_loaded=model_loaded,
            version=version
        )
        
        status_code = 200 if status == "healthy" else 503
        return jsonify(response), status_code
        
    except Exception as e:
        return jsonify({
            'status': 'unhealthy',
            'error': str(e)
        }), 503


@health_bp.route('/health/ready', methods=['GET'])
def readiness():
    """Readiness probe - kiểm tra model đã sẵn sàng chưa."""
    try:
        config = Config()
        model_loader = ModelLoader(config)
        
        # Model phải được load thành công
        model = model_loader.get_model()
        
        if model is not None:
            return jsonify({'ready': True}), 200
        else:
            return jsonify({'ready': False, 'reason': 'Model not loaded'}), 503
            
    except Exception as e:
        return jsonify({'ready': False, 'reason': str(e)}), 503


@health_bp.route('/health/live', methods=['GET'])
def liveness():
    """Liveness probe - server còn sống không."""
    return jsonify({'alive': True}), 200


@health_bp.route('/info', methods=['GET'])
def info():
    """Server info - trả về thông tin config."""
    try:
        config = Config()
        
        return jsonify({
            'server': {
                'version': config.get('server.version', '1.0.0'),
                'host': config.get('server.host', '0.0.0.0'),
                'port': config.get('server.port', 5000)
            },
            'model': {
                'path': config.get('model.path'),
                'device': config.get('model.device', 'cpu'),
                'confidence_threshold': config.get('model.confidence_threshold', 0.25)
            },
            'classes': config.get('classes', []),
            'bin_mapping': config.get('bin_mapping', {})
        }), 200
        
    except Exception as e:
        return jsonify({'error': str(e)}), 500
