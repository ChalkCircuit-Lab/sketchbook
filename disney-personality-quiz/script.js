// This file contains the JavaScript logic for the animal personality quiz.

document.addEventListener('DOMContentLoaded', () => {
    const form = document.getElementById('quiz-form');
    const submitBtn = document.getElementById('submit-btn');
    const resultContainer = document.getElementById('result');
    
    fetch('data/disney-questions.csv')
        .then(response => response.text())
        .then(data => {
            const questions = parseCSV(data);
            loadQuestions(questions);
        });

    submitBtn.addEventListener('click', (event) => {
        event.preventDefault();
        const formData = new FormData(form);
        let score = 0;

        // Process form data and calculate score
        formData.forEach((value, key) => {
            score += parseInt(value);
        });

        // Determine result based on score
        let result;
        if (score < 10) {
            result = 'You are Mickey Mouse!';
        } else if (score < 20) {
            result = 'You are Donald Duck!';
        } else {
            result = 'You are Goofy!';
        }

        // Display result
        resultContainer.textContent = result;
    });
});