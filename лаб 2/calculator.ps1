# Simple Calculator
while ($true) {
    Write-Host "`n=== SIMPLE CALCULATOR ===" -ForegroundColor Green
    Write-Host "1. plus  2. minus  3. *        4. /"
    Write-Host "5. ^     6. koren  7. sin      8. cos"
    Write-Host "9. tan   10. log   11. modul   0. exit"
    
    $choice = Read-Host "`nSelect operation"
    
    if ($choice -eq "0") { 
        Write-Host "Poka Poka!" -ForegroundColor Yellow
        break 
    }
    
    try {
        switch ($choice) {
            "1" {
                $a = [double](Read-Host "First number")
                $b = [double](Read-Host "Second number")
                Write-Host "Result: $a + $b = $($a + $b)" -ForegroundColor Green
            }`
            "2" {
                $a = [double](Read-Host "First number")
                $b = [double](Read-Host "Second number")
                Write-Host "Result: $a - $b = $($a - $b)" -ForegroundColor Green
            }
            "3" {
                $a = [double](Read-Host "First number")
                $b = [double](Read-Host "Second number")
                Write-Host "Result: $a * $b = $($a * $b)" -ForegroundColor Green
            }
            "4" {
                $a = [double](Read-Host "Delimoe")
                $b = [double](Read-Host "Delitel")
                if ($b -eq 0) { throw "Division by zero!" }
                Write-Host "Result: $a / $b = $($a / $b)" -ForegroundColor Green
            }
            "5" {
                $a = [double](Read-Host "Osnovanie")
                $b = [double](Read-Host "Stepen")
                Write-Host "Result: $a ^ $b = $([Math]::Pow($a, $b))" -ForegroundColor Green
            }
            "6" {
                $a = [double](Read-Host "Chislo")
                $b = [double](Read-Host "Stepen kornya")
                Write-Host "Result: koren $b iz $a = $([Math]::Pow($a, 1/$b))" -ForegroundColor Green
            }
            "7" {
                $a = [double](Read-Host "Ugly v gradusah")
                Write-Host "Result: sin($a°) = $([Math]::Sin($a * [Math]::PI / 180))" -ForegroundColor Green
            }
            "8" {
                $a = [double](Read-Host "Ugly v gradusah")
                Write-Host "Result: cos($a°) = $([Math]::Cos($a * [Math]::PI / 180))" -ForegroundColor Green
            }
            "9" {
                $a = [double](Read-Host "Ugly v gradusah")
                Write-Host "Result: tan($a°) = $([Math]::Tan($a * [Math]::PI / 180))" -ForegroundColor Green
            }
            "10" {
                $a = [double](Read-Host "Chislo")
                Write-Host "Result: log10($a) = $([Math]::Log10($a))" -ForegroundColor Green
            }
            "11" {
                $a = [double](Read-Host "Delimoe")
                $b = [double](Read-Host "Delitel")
                Write-Host "Result: $a % $b = $($a % $b)" -ForegroundColor Green
            }
            default { 
                Write-Host "Error!" -ForegroundColor Red 
            }
        }
    }
    catch {
        Write-Host "Error: $($_.Exception.Message)" -ForegroundColor Red
    }
    
    $cont = Read-Host "`nContinue? (y/n)"
    if ($cont -ne "y" -and $cont -ne "Y") { 
        Write-Host "Poka Poka!" -ForegroundColor Yellow
        break 
    }
}