// Объединённый код валидации формы и анимаций
document.addEventListener('DOMContentLoaded', function() {
    const form = document.getElementById('contact-form');
    if (form) {
        const nameInput = document.getElementById('name');
        const emailInput = document.getElementById('email');
        const messageInput = document.getElementById('message');
        const nameError = document.getElementById('name-error');
        const emailError = document.getElementById('email-error');
        const messageError = document.getElementById('message-error');
        const successMessage = document.getElementById('success-message');
        const submitBtn = document.querySelector('.submit-btn');
        
        let isSubmitting = false;
        
        // Валидация имени
        if (nameInput) {
            nameInput.addEventListener('input', function() {
                validateName();
            });
        }
        
        // Валидация email
        if (emailInput) {
            emailInput.addEventListener('input', function() {
                validateEmail();
            });
        }
        
        // Валидация сообщения
        if (messageInput) {
            messageInput.addEventListener('input', function() {
                validateMessage();
            });
        }
        
        function validateName() {
            if (!nameInput) return true;
            if (nameInput.value.trim() === '') {
                showError(nameError, 'Имя обязательно для заполнения');
                return false;
            }
            hideError(nameError);
            return true;
        }
        
        function validateEmail() {
            if (!emailInput) return true;
            const emailRegex = /^[^\s@]+@[^\s@]+\.[^\s@]+$/;
            if (!emailRegex.test(emailInput.value.trim())) {
                showError(emailError, 'Введите корректный email');
                return false;
            }
            hideError(emailError);
            return true;
        }
        
        function validateMessage() {
            if (!messageInput) return true;
            const maxLength = 500;
            const currentLength = messageInput.value.length;
            
            if (currentLength === 0) {
                showError(messageError, 'Сообщение не может быть пустым');
                return false;
            }
            
            if (currentLength > maxLength) {
                showError(messageError, `Сообщение должно быть не более ${maxLength} символов (сейчас: ${currentLength})`);
                return false;
            }
            
            hideError(messageError);
            return true;
        }
        
        function showError(element, message) {
            if (!element) return;
            element.textContent = message;
            element.style.display = 'block';
        }
        
        function hideError(element) {
            if (!element) return;
            element.style.display = 'none';
        }
        
        function showSuccess(message) {
            if (!successMessage) return;
            successMessage.textContent = message;
            successMessage.style.display = 'block';
            
            // Скрыть сообщение через 5 секунд
            setTimeout(() => {
                successMessage.style.display = 'none';
            }, 5000);
        }
        
        function disableSubmit() {
            if (!submitBtn) return;
            submitBtn.disabled = true;
            submitBtn.textContent = 'Отправка...';
            isSubmitting = true;
        }
        
        function enableSubmit() {
            if (!submitBtn) return;
            submitBtn.disabled = false;
            submitBtn.textContent = 'Отправить сообщение';
            isSubmitting = false;
        }
        
        // Обработка отправки формы
        form.addEventListener('submit', async function(event) {
            event.preventDefault();
            
            if (isSubmitting) return;
            
            // Сброс сообщений
            hideError(nameError);
            hideError(emailError);
            hideError(messageError);
            if (successMessage) successMessage.style.display = 'none';
            
            // Валидация
            const isValid = validateName() && validateEmail() && validateMessage();
            
            if (!isValid) return;
            
            disableSubmit();
            
            try {
                const formData = {
                    name: nameInput.value.trim(),
                    email: emailInput.value.trim(),
                    message: messageInput.value.trim()
                };
                
                const response = await fetch('/api/contact', {
                    method: 'POST',
                    headers: {
                        'Content-Type': 'application/json',
                    },
                    body: JSON.stringify(formData)
                });
                
                const result = await response.json();
                
                if (result.success) {
                    showSuccess('Сообщение успешно отправлено! Спасибо за ваше обращение.');
                    form.reset();
                } else {
                    showError(messageError, result.message || 'Произошла ошибка при отправке');
                }
            } catch (error) {
                console.error('Ошибка:', error);
                showError(messageError, 'Произошла ошибка при отправке. Попробуйте позже.');
            } finally {
                enableSubmit();
            }
        });
    }

    // Анимация при наведении на карточки проектов
    const projectCards = document.querySelectorAll('.project-card');
    projectCards.forEach(card => {
        card.addEventListener('mouseenter', function() {
            this.style.transform = 'translateY(-8px)';
        });
        
        card.addEventListener('mouseleave', function() {
            this.style.transform = 'translateY(0)';
        });
    });
    
    const fadeElements = document.querySelectorAll('.skill-item, .experience-item, .contact-link');
    fadeElements.forEach((el, index) => {
        el.style.opacity = '0';
        el.style.transform = 'translateY(10px)';
        el.style.transition = 'opacity 0.5s ease, transform 0.5s ease';
        
        setTimeout(() => {
            el.style.opacity = '1';
            el.style.transform = 'translateY(0)';
        }, 100 + (index * 100));
    });
});